import {ComponentBase} from '../../drjs/browser/component.js';
import domEvent from '../../drjs/browser/dom-event.js';
import DOMEvent from '../../drjs/browser/dom-event.js';
import DOM from '../../drjs/browser/dom.js'
import HTMLTemplate from '../../drjs/browser/html-template.js'

const notificationsHTML= `
<div class="notifications">
</div>
`;

const noticeHTML= `
<div class="notice">
    <span class='text'></span>
</div>
`;

export class Notifications {
    constructor(selector) {

        this.notificationTemplate = new HTMLTemplate(notificationsHTML);
        this.notifications = this.notificationTemplate.getNodes()[0];
        document.body.appendChild(this.notifications);
        this.hide();
    }

    notify(text,seconds=5){
        var notice = new HTMLTemplate(noticeHTML);
        var note = notice.fill({'.text':text})[0];
        this.notifications.appendChild(note);
        this.show();
        setTimeout(()=>this.removeNotice(note),5000);
        return note;
    }

    removeNotice(notice) {
        DOM.remove(notice);
        var first = DOM.first(this.notifications,'.notice');
        if (first == null) {
            this.hide();  // no notices left
        }
    }
    show() {
        DOM.show(this.notifications);
    }

    hide() {
        DOM.hide(this.notifications);
    }

}

var notifications = new Notifications();

export default notifications;