export default {
	log() {
		__ccLog(__mGetStr(arguments), "log");
	},
	info() {
		__ccLog(__mGetStr(arguments), "info");
	},
	warning() {
		__ccLog(__mGetStr(arguments), "warning");
	},
	error() {
		__ccLog(__mGetStr(arguments), "error");
	}/*,
	getStr(args) {
		const arrLen = args.length;
		let str = "";
		for (let i = 0; i < arrLen; i++) {
//			const v = args[i];
//			val = typeof v !== "object" || v === self ? v : JSON.stringify(v);
//			str += i !== 0 ? ", " + val : val;
			str += i !== 0 ? ", " + args[i] : args[i];
		}
		return str;
	}*/
};
