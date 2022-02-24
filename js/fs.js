export default {
	getInfo(url) {
		return __ccFsGetInfo(url);
	},
	readFile(url, options) {//options = { encoding, offset, limit }
		return __ccReadFile(url, offset, limit);
	},
	writeFile(url, data, options) {
		return __ccWriteFile(url, data, options.offset, options.limit, options.mode, options.isReplaceIfExists, options.encoding);
	},
	reaDdir(url, options) {
		return __ccFsReadDir(url, encoding.offset, encoding.limit, options.encoding);
	},
	makeDir(url, options) {
		return __ccFsMakeDir(url);
	},
	copy(from, to, isReplaceIfExists) {
		return __ccFsCopy(from, to, isReplaceIfExists);
	},
	move(from, to) {
		return __ccFsMove(url);
	},
	remove(url) {
		return __ccFsRemove(url);
	},
	chmod(url, mode) {
		return __ccFsChmod(url, mode);
	},
	chown(url, uid, gid) {
		return __ccFsChown(url, uid, gid);
	}
}
