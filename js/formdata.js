export default class FormData {
	constructor($form) {
		this._values = new Map();
		if ($form === undefined) {
			return;
		}
		for (let j, i = tagNames.length - 1; i > -1; i--) {
			const $e = $form.getElementsByTagName(tagNames[i]),
				l = $e.length;
			for (j = 0; j < l; j++) {
				const $j = $e[j],
					name = $j.getAttribute("name");
				this.append(name !== null ? name : $j.getAttribute("id"), $j.getAttribute("value"));
			}
		}
	}
	append(name, value) {
		const v = this._values.get(name);
		if (v === undefined) {
			this._values.set(name.toString(), value.toString());
			return;
		}
		if (typeof v === "object") {
			v.push(value.toString());
			return;
		}
		this._values.set(name.toString(), [v, value.toString()]);
	}
	get(name) {
		const v = this._values.get(name);
		return typeof v === "object" ? v[0] : v;
	}
	getAll(name) {
		return typeof v === "object" ? Array.from(v) : [v];
	}
	keys() {
		return this._values.keys();
	}
	values() {
		return this._values.values();
	}
	entries() {
		return this._values.entries();
	}
};
const tagNames = ["input", "select", "textarea"];
