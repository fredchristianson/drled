import {ComponentBase} from '../../drjs/browser/component.js';
import assert from '../../drjs/assert.js';
import util from '../../drjs/util.js';
import DOM from '../../drjs/browser/dom.js';
import  DOMEvent from '../../drjs/browser/dom-event.js';
import Logger from '../../drjs/logger.js';
import notice from './notify.js';

const log = Logger.create("ScriptEditor");

import {HtmlTemplate, HtmlValue,TextValue,AttributeValue,DataValue } from '../../drjs/browser/html-template.js';
import Modal from './modal.js';

export class ScriptEditorComponent extends ComponentBase{
    constructor(selector, strips, htmlName='script') {
        super(selector,htmlName);
        this.strip = null;
        DOMEvent.listen('singleStripSelection',this.onStripSelected.bind(this));
        DOMEvent.listen('click','.script .save-script',this.onSave.bind(this));
        DOMEvent.listen('click','.script .load-script',this.onLoad.bind(this));
        DOMEvent.listen('click','.script .delete-script',this.onDelete.bind(this));
        DOMEvent.listen('click','.script .copy-script',this.onCopy.bind(this));
        DOMEvent.listen('change','.script .select-script',this.onSelect.bind(this));
        DOMEvent.listen('change','.script .script-list input',this.onScriptSelect.bind(this));

        DOMEvent.listen('input','.script input[name="name"]',this.onChange.bind(this));
        DOMEvent.listen('input','.script textarea',this.onChange.bind(this));

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

    onChange() {
        this.changed = true;
        DOM.setProperty('.script .save-script','disabled',false);
        DOM.setProperty('.script .load-script','disabled',false);

        notice.notify("change ");
    }

    updateUI() {
        log.debug("updatestate "+this.selectedScripts.length);
        var selCount = this.selectedScripts.length;
        var single =  selCount == 1;
        var multiple =  selCount > 1;
        DOM.show('.script .single-script',single);
        DOM.show('.script .save-script',single || multiple);
        DOM.show('.script .load-script',single || multiple);
        DOM.show('.script .delete-script',single || multiple);
        DOM.show('.script .copy-script',single || multiple);

        DOM.show('.script .save-script',single || multiple);
        DOM.show('.script .load-script',single || multiple);

        DOM.setProperty('.script .save-script','disabled',true);
        DOM.setProperty('.script .load-script','disabled',true);


    }

    async loadScript(name) {
        var script = await this.strip.getScript(name);
        DOM.setValue(".single-script input[name='name']",name);
        var text = JSON.stringify(script,null,2);
        DOM.setValue('#script-text',text);
        this.changed = false;
        this.updateUI();
    }

    async onStripSelected(strip){
        this.strip = strip;
        if (strip == null) {
            DOM.hide('.script');
            return;
        }
        log.debug("strip selected ",strip.getName());
        DOM.show('.script');

        this.config = await this.strip.getConfig();
        var scriptTemplate = new HtmlTemplate(DOM.first("#script-item"));
        this.scriptList.innerHTML = '';
        if (this.config && this.config.scripts) {
            this.config.scripts.forEach(item=>{
                const values = {
                    '.name': item,
                    '.item': new AttributeValue('data-name',item)
                };
                var row = scriptTemplate.fill(values);
                DOM.append(this.scriptList,row);
            });
        }

        DOM.setValue(this.text,'');
        this.updateUI();
        //DOM.setOptions(this.select,config.scripts,"--select--");
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

    onSave(element,event) {
        var name = DOM.getValue(this.name);
        log.debug("save ",this.nameElement);
        var val = DOM.getValue("#script-text");
        this.strip.saveScript(name,val);
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
        return DOMEvent.HANDLED;
    }

    async onCopy(element,event) {
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

}

export default ScriptEditorComponent;