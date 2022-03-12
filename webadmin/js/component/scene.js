import {ComponentBase} from '../../drjs/browser/component.js';
import assert from '../../drjs/assert.js';
import util from '../../drjs/util.js';
import DOM from '../../drjs/browser/dom.js';
import DOMEvent from '../../drjs/browser/dom-event.js';
import Logger from '../../drjs/logger.js';
import notice from './notify.js';
import ENV from '../../drjs/env.js'
const log = Logger.create("Scene");

import {HtmlTemplate, HtmlValue,TextValue,AttributeValue,DataValue } from '../../drjs/browser/html-template.js';

export class SceneComponent extends ComponentBase{
    constructor(selector, htmlName='scene') {
        super(selector,htmlName);
        this.pageSize = 10;
        this.listeners = [];
    }

    async onAttached(elements,parent){
        this.listeners = [
            DOMEvent.listen('stripSelection',this.onScriptSelect.bind(this)),
            DOMEvent.listen('change','.script .script-list input',this.onScriptSelect.bind(this))
        ]
            
    }

    detach() {
        DOMEvent.removeListener(this.listeners);
        this.listeners= [];
    }

    async onScriptSelect(changed, component) {
        var sel = ENV.THEAPP.getSelectedStrips();
        if (sel == null || sel.length == 0) { return;}
        var first = sel[0];
        var scripts = await first.getScripts();
        for(const strip of sel) {
            if (strip != first && scripts.length>0) {
                scripts = util.intersect(scripts,await strip.getScripts());
            }
        };
        var buttons = DOM.first('.scene-controls .buttons');
        buttons.innerHTML = '';
        scripts.forEach(script=>{
            var button = new HtmlTemplate(DOM.first('#scene-button'));
            var values = {
                '.scene-select': [script, new DataValue('name',script)]
            };
            DOM.append(buttons,button.fill(values));
        });
    }
}

export default SceneComponent;