import Application from '../drjs/browser/application.js';
import Logger from '../drjs/logger.js';
import page from '../drjs/browser/page.js';
import httpRequest from '../drjs/browser/http-request.js';

import  util  from '../drjs/util.js';
import DOMEvent from "../drjs/browser/dom-event.js";
import DOM from "../drjs/browser/dom.js";

import { DomLogWriter } from '../drjs/browser/log-writer-dom.js';
import notice from './component/notify.js';

const log = Logger.create("LedApp");



import HeaderComponent from './component/header.js';
import WhiteComponent from './component/white.js';
import SceneComponent from './component/scene.js';
import ColorComponent from './component/color.js';
import FooterComponent from './component/footer.js';
import ConfigComponent from './component/config.js';
import StripComponent from './component/strip.js';
import ScriptEditorComponent from './component/script-editor.js';
import { LOG_LEVEL } from '../drjs/log-message.js';

const PAGE_MAIN_COMPONENT = {
    "index" : WhiteComponent,
    "home" : WhiteComponent,
    "view" : WhiteComponent,
    "color" : ColorComponent,
    "scene" : SceneComponent,
    "white" : WhiteComponent,
    "scripts": ScriptEditorComponent,
    "config" : ConfigComponent
};

const LED_STRIPS = [
    {name: 'Living Room',host:"lr.400ggp.com"},
    {name: 'Dining Room',host:"dr.400ggp.com"},
    {name: 'Kitchen Cupboard',host:"kc.400ggp.com"},
    {name: 'Kitchen Floor',host:"kf.400ggp.com"},
    {name: 'Lanai',host:"lanai.400ggp.com"},
    {name: 'Hall',host:"hall.400ggp.com"},
    {name: 'Test',host:"192.168.10.187"},
    {name: 'Demo',host:"192.168.10.130"}
];

var NEXT_STRIP_ID=1;
class Strip {
    constructor(name,host) {
        this.name = name;
        this.host = host;
        this.id = NEXT_STRIP_ID++;
        this.config = null;
        this.selected = false;
    }

    getId() { return this.id;}
    getName() { return this.name;}
    getHost() { return this.host;}

    setSelected(sel) {this.selected=sel;}
    isSelected() { return this.selected;}

    async getScripts() {
        var cfg = await this.getConfig();
        var scripts = [];
        if (cfg&&cfg.scripts){
            scripts = scripts.concat(cfg.scripts);
        }
        return scripts;
    }
    async getConfig(refresh=false){
        if (this.config && !refresh) {
            return this.config;
        }
        var host = this.host;
        var url = `://${host}/api/config`;
        var configJson = await this.apiGet(url);
        try {
            this.config = configJson; 
        } catch(err) {
            log.error("JSON error on config ",configJson);
            this.config = {};
        }
        return this.config;
    }

    async saveConfig(config) {
        var host = this.host;
        var url = `://${host}/api/config`;
        return await this.apiPost(url,config);
    }

    async saveScript(name,script) {
        var host = this.host;
        var url = `://${host}/api/script/${name}`;
        this.config = null;
        return await this.apiPost(url,script);
    }

    async getScript(name){
        var host = this.host;
        var url = `://${host}/api/script/${name}`;
        var script = await this.apiGet(url);
        return script;
    }

    async deleteScript(name){
        var host = this.host;
        var url = `://${host}/api/script/${name}`;
        var script = await this.apiDelete(url);
        return script;
    }

    async run(script){
        // deprecated.  use runScript()
        return this.runScript(script);
    }

    async runScript(script){
        var host = this.host;
        var url = `://${host}/api/run/${script}`;
        log.debug("url: "+url);
        await this.apiGet(url);
    }

    async setColor(hue,sat,light){
        var host = this.host;
        var url = `://${host}/api/color?hue=${hue}&saturation=${sat}&lightness=${light}`;
        log.debug("url: "+url);
        await this.apiGet(url);
    }

    async setWhite(brightness){
        var host = this.host;
        var url = `://${host}/api/on?level=${brightness}`;
        log.debug("url: "+url);
        await this.apiGet(url);
    }

    async setOff(){
        var host = this.host;
        var url = `://${host}/api/off`;
        log.debug("url: "+url);
        await this.apiGet(url);
    }

    async apiGet(api,params=null) {
        var note = notice.notify("GET: "+api);
        var response = await this.sendApi(httpRequest.get,api,params,"json");
        note.innerText = note.innerText + " - " +(response ? "success":"failed");
        return response;
    };
    
    
    async apiDelete(api,params=null) {
        var note = notice.notify("DELETE: "+api);
        var response = await this.sendApi(httpRequest.delete,api,params,"json");
        note.innerText = note.innerText + " - " +(response ? "success":"failed");
        return response;
    };
    
    
    async apiPost(api,body) {
        try {
            log.debug("POST ",api);
            var note = notice.notify("POST: "+api);

            var response = await httpRequest.post(api,body,"json");
            if (response && response.success) {
                return response.data || {};
            }
            log.error("failed to POST ",api,JSON.stringify(response));
            return null;            
        } catch(ex){
            log.error("failed to POST ",api,ex);
            return null;
        }
    };

    async sendApi(method,api,params,type){
        try {
            var response = await method.call(httpRequest,api,params,type);
            if (response && response.success) {
                return response.data || {};
            }
            return null;
        } catch(ex){
            log.error("API failed ",api,ex);
            return null;
        }
    }
}

export class LedApp extends Application {
    constructor() {
        super("LED App");
        
    }

    initialize() {
        new DomLogWriter('#log-container .messages',LOG_LEVEL.WARN);
        log.debug("test");
        var gotoPage = location.hash.substr(1);
        page.setDefaultPage('white');
        this.header = new HeaderComponent('header');
        this.main = this.loadMainContent('white');

        this.strips = LED_STRIPS.map(def=>{
            return new Strip(def.name,def.host);
        });
        this.stripComponent = new StripComponent('#strips',this.strips);
        this.footer = new FooterComponent('footer');
        DOMEvent.listen('click','#main-nav a',this.onNav.bind(this));
        DOMEvent.listen('componentLoaded',this.onComponentLoaded.bind(this));
        DOMEvent.listen('click','a[href="#select-all-strips"]',this.selectAllStrips.bind(this));
        DOMEvent.listen('click','a[href="#select-no-strips"]',this.selectNoStrips.bind(this));
        DOMEvent.listen('change','.white-controls input.slider',this.setWhite.bind(this));
        DOMEvent.listen('click','.white-on ',this.setWhite.bind(this));
        DOMEvent.listen('click','.color-on ',this.setColor.bind(this));
        DOMEvent.listen('click','.scene-off ',this.setOff.bind(this));
        DOMEvent.listen('change','.color-controls input.slider',this.setColor.bind(this));
        DOMEvent.listen('click','#reload-page',this.reload.bind(this));
        DOMEvent.listen('click','#toggle-log',this.toggleLog.bind(this));
        DOMEvent.listen('click','#log-clear',this.clearLog.bind(this));
        DOMEvent.listen('click','.scene-select',this.selectScene.bind(this));
        DOMEvent.listen('click','.check-all',this.checkAll.bind(this));

        if (gotoPage && gotoPage.length>0) {
            this.loadMainContent(gotoPage);
        } 

    }

    getStrips() { return this.strips;}
    getStripById(id) { return this.strips.find(s=>s.id==id);}
    getSelectedStrips() { return this.strips.filter(s=>s.isSelected())}


    clearLog() {
        DOM.removeChildren("#log-container .messages");
    }
    toggleLog() {
        DOM.toggleClass('#log-container','hidden');
        DOM.toggleClass('#toggle-log','off');   
    }

    reload() {
        location.reload(true);
    }

        
    selectScene(element) {
        var sel = DOM.find('#strips input:checked');
        var script = DOM.getData(element,"name");
        log.debug("scene",script);
        this.strips.filter(strip=> this.stripComponent.isSelected(strip)).forEach(strip=>{
            log.debug("select scene ",strip.getName());
            strip.run(script);
        });

    }    

    checkAll(element) {
        var target = DOM.getData(element,"target");
        var checked = DOM.getData(element,"checked")==="true";
        var checks = DOM.find(target + " input");
        checks.forEach((cbox)=>{
            DOM.setProperty(cbox,"checked",checked);
        });
        return DOMEvent.HANDLED;
        

    }    

    setColor() {
        var hue = DOM.getIntValue('#color-hue');
        var sat = DOM.getIntValue('#color-saturation');
        var light = DOM.getIntValue('#color-lightness');

        var sel = DOM.find('#strips input:checked');
        this.strips.filter(strip=> this.stripComponent.isSelected(strip)).forEach(strip=>{
            log.debug("set color ",strip.getName());
            strip.setColor(hue,sat,light);
        });

    }    
    
    setWhite() {
        var val = DOM.getIntValue('.white-controls .slider');
        this.strips.filter(strip=> this.stripComponent.isSelected(strip)).forEach(strip=>{
            log.debug("set white ",strip.getName());
            strip.setWhite(val);
        });

    }    
    
    setOff() {
        log.debug("turn off");
        this.strips.filter(strip=> this.stripComponent.isSelected(strip)).forEach(strip=>{
            log.debug("set off ",strip.getName());
            strip.setOff();
        });


    }

    selectAllStrips() {
        DOM.check('.strip-list input',true);
    }
    
    selectNoStrips() {
        DOM.uncheck('.strip-list input');
    }

    onComponentLoaded(component) {
        log.info("loaded component ",component.getName());
        this.setNav();
    }
    onNav(element,event) {
        var href = element.getAttribute("href");
        if (href != null && href[0] == '#') {
            var sel = href.substr(1);
            if (sel) {
                this.loadMainContent(sel);
                event.stopPropagation();
                event.preventDefault();
            }
    
        }
    }

    loadMainContent(page = null) {
        if (util.isEmpty(page)){
            page = "white";
        }
        this.currentPage = page;
        log.info("load component ",page);
        location.hash=page;
        const componentHandler = PAGE_MAIN_COMPONENT[page] || WhiteComponent;
        
        this.mainComponent = new componentHandler('#main-content');
    }

    setNav() {
        DOM.removeClass("#main-nav a",'active');
        DOM.addClass("a[href='#"+this.currentPage+"']",'active');

    }


}

export default LedApp;