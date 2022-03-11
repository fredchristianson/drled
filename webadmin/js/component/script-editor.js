import {ComponentBase} from '../../drjs/browser/component.js';
import assert from '../../drjs/assert.js';
import util from '../../drjs/util.js';
import DOM from '../../drjs/browser/dom.js';
import  DOMEvent from '../../drjs/browser/dom-event.js';
import Logger from '../../drjs/logger.js';
import notice from './notify.js';
import ENV from '../../drjs/env.js'

const log = Logger.create("ScriptEditor");

import {HtmlTemplate, HtmlValue,PropertyValue,AttributeValue,DataValue } from '../../drjs/browser/html-template.js';
import Modal from './modal.js';
import domEvent from '../../drjs/browser/dom-event.js';

export class ScriptEditorComponent extends ComponentBase{
    constructor(selector, htmlName='script') {
        super(selector,htmlName);
        this.strip = null;
        this.listeners = [
            DOMEvent.listen('singleStripSelection',this.onStripSelected.bind(this)),
            DOMEvent.listen('click','.script .save-script',this.onSave.bind(this)),
            DOMEvent.listen('click','.script .load-script',this.onLoad.bind(this)),
            DOMEvent.listen('click','.script .run-script',this.onRun.bind(this)),
            DOMEvent.listen('click','.script .delete-script',this.onDelete.bind(this)),
            DOMEvent.listen('click','.script .copy-script',this.onCopy.bind(this)),
            DOMEvent.listen('change','.script .select-script',this.onSelect.bind(this)),
            DOMEvent.listen('change','.script .script-list input',this.onScriptSelect.bind(this)),
            DOMEvent.listen('click','.script .refresh-scripts',this.onRefreshScripts.bind(this)),
            DOMEvent.listen('input','.script input[name="name"]',this.onChange.bind(this)),
            DOMEvent.listen('input','.script textarea',this.onChange.bind(this))
        ]

        this.debouncedUpdateUI = util.debounce(this.updateUI.bind(this),200);
        this.selectedScripts = [];
        this.updateUI();
        this.changed = false;
    }

    async onScriptSelect(element) {
        this.selectedScripts = DOM.find('.script-list input:checked').map(i=>{
            return i.parentNode.innerText;
        });
        if (this.selectedScripts.length==1) {
            this.loadScript(this.selectedScripts[0]);
        }
        this.debouncedUpdateUI();
    }

    onDetach() {
        DOMEvent.removeListener(this.listeners);
    }

    onChange() {
        this.changed = true;
        DOM.setProperty('.script .save-script','disabled',false);
        DOM.setProperty('.script .load-script','disabled',false);

  
    }

    updateUI() {
        log.debug("updatestate "+this.selectedScripts.length);
        var selCount = this.selectedScripts.length;
        var single =  selCount == 1;
        var multiple =  selCount > 1;
        DOM.show('.script .single-script',single);
        DOM.show('.script .run-script',single || multiple);
        DOM.show('.script .save-script',single || multiple);
        DOM.show('.script .load-script',single || multiple);
        DOM.show('.script .delete-script',single || multiple);
        DOM.show('.script .copy-script',single || multiple);

        DOM.show('.script .save-script',single || multiple);
        DOM.show('.script .load-script',single || multiple);

        DOM.setProperty('.script .load-script','disabled',true);
        DOM.setProperty('.script .save-script','disabled',true);
        DOM.setProperty('.script .run-script','disabled',!single);


    }

    async loadScript(name) {
        var script = await this.strip.getScript(name);
        DOM.setValue(".single-script input[name='name']",name);
        var text = JSON.stringify(script,null,2);
        DOM.setValue('#script-text',text);
        this.changed = false;
        this.updateUI();
    }

    async onRefreshScripts() {
        return await this.onStripSelected(this.strip);
    }
    async onStripSelected(strip){
        this.strip = strip;
        if (strip == null) {
            DOM.hide('.script');
            return;
        }
        log.debug("strip selected ",strip.getName());
        DOM.show('.script');
        DOM.setValue(this.text,'');
        await this.loadScripts();
        
        return DOMEvent.HANDLED;
        //DOM.setOptions(this.select,config.scripts,"--select--");
    }

    async loadScripts(selected=null) {
        this.config = await this.strip.getConfig();
        var scriptTemplate = new HtmlTemplate(DOM.first("#script-item"));
        this.scriptList.innerHTML = '';
        if (this.config && this.config.scripts) {
            this.config.scripts.forEach(item=>{
                const values = {
                    '.name': item,
                    '.item': new AttributeValue('data-name',item),
                    '.check': [new AttributeValue('value',item),
                                new PropertyValue('checked',selected != null && item == selected)]
                };
                var row = scriptTemplate.fill(values);
                DOM.append(this.scriptList,row);
            });
        }

        this.updateUI();
    }



    async onSelect(element,event) {
        var name = DOM.getValue(element);
        var text = "";
        if (name != null) {
            DOM.setValue(this.name,name);
            text = await this.strip.getScript(name);
        }
        DOM.setValue(this.text,text);
        
        
        return DOMEvent.HANDLED;
    }

    async onSave(element,event) {
        var name = DOM.getValue(this.name);
        log.debug("save ",this.nameElement);
        var val = DOM.getValue("#script-text");
        await this.strip.saveScript(name,val);
        
        log.debug("save complete");
        this.loadScripts(name);
        return DOMEvent.HANDLED;
    }

    async onRun(element,event) {
        await this.strip.runScript(this.selectedScripts[0]);
        return DOMEvent.HANDLED;
    }

    async onLoad(element,event) {
        await this.loadScript(this.selectedScripts[0]);
        return DOMEvent.HANDLED;
    }

    async onDelete(element,event) {
        var dlg = new HtmlTemplate(DOM.first('.script .delete-confirmation'),{'.name':this.selectedScripts});
        var modal = new Modal(dlg);
        var response = await modal.show();
        log.debug("response: "+response);
        if (response == 'yes'){
            await Promise.all(this.selectedScripts.map(async (script)=>{
                return this.strip.deleteScript(script);
            }));
            // reload script list
            log.debug("delete complete");

            this.onStripSelected(this.strip);
        }
        return DOMEvent.HANDLED;
    }

    async onCopy(element,event) {
        const scripts = this.selectedScripts.map(s=>{ return {'.script-name':s};});
        const to = ENV.THEAPP.getStrips().map(strip=> {
            return {
            '.name': strip.name,
            'li': new AttributeValue('data-name',strip.name),
            'input': [new AttributeValue('value',strip.id),new PropertyValue('strip',strip)]
            }
        });
        var dlg = new HtmlTemplate(DOM.first('.script .copy-to'),{'li.sel':to, ".script-name":scripts});
        var modal = new Modal(dlg);
        var response = await modal.show();
        log.debug("response: "+response);
        if (response == 'yes'){
            var content = modal.getDOM();
            var checks = DOM.find(content,'input[type="checkbox"]:checked');
            for(const script of this.selectedScripts) {
                log.debug("get script ",script);
                var text = await this.strip.getScript(script);
                log.debug("got script ",script);
                for (const check of checks){
                    var stripId = check.value;
                    var strip = this.getStripById(stripId);
                    log.debug("\tsend script to ",strip.name);
                    await strip.saveScript(script,text);
                    log.debug("\tsent script to ",strip.name);
                }
            }
            log.debug("copy to ",checks.length," controllers");

            //this.onStripSelected(this.strip);
        }
        return DOMEvent.HANDLED;
    }

    async onAttached(elements,parent){
        this.elements = elements;
        this.text = DOM.first(parent,'#script-text');
        this.name = DOM.first(parent,'.script [name="name"]');
        this.select = DOM.first(parent,'.select-script');
        this.scriptList = DOM.first(parent,'.script-list');
        
        this.loaded = true;
        this.onStripSelected(this.strip);

    }

    getStripById(id){
        return ENV.THEAPP.getStripById(id);
    }

}

export default ScriptEditorComponent;