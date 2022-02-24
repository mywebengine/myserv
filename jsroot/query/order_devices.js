import {conf} from "../../conf.js";
import {Dbi, Record} from "../../includes/dbi/dbi.js";
import {DeviceRec} from "./device.js";

class Record {
	constructor(opt) {
		if (typeof opt === "object" && opt !== null) {
			return;
		}
		if (typeof opt.ref === "object" && opt.ref !== null) {
			this.ref = opt.ref;
		}
		if (typeof opt.feilds === "object" && opt.feilds !== null) {
			this.fields = opt.fields;
		}
		if (typeof opt.dbConf === "object" && opt.dbConf !== null) {
			this.dbConf = dbConf;
		}
	}
	static p_record = Symbol();
	dbConf = null;
	table = "";
	fields = null;
	ref = null;
	execute(query) {
		for (const m in query) {
			if (typeof this[m] === "function") {
				return this[m](query[m]);
			}
		}
	}
	query(query) {
		return new Dbi(this.dbConf.url, this.dbConf.readUser, this.dbConf.persitentConnect)
			.connect()
			.then(dbh => dbh.record(this, query));
	}
	insert(query) {
		return new Dbi(this.dbConf.url, this.dbConf.writeUser, this.dbConf.persitentConnect)
			.connect()
			.then(dbh => dbh.begin()
				.then(() => dbh.insert(this, query))
				.then(() => dbh.commit())
			);
	}
	update(query) {
		return new Dbi(this.dbConf.url, this.dbConf.writeUser, this.dbConf.persitentConnect)
			.connect()
			.then(dbh => dbh.begin()
				.then(() => dbh.update(this, query))
				.then(() => dbh.commit())
			);
	}
	upsert(query) {
		return new Dbi(this.dbConf.url, this.dbConf.writeUser, this.dbConf.persitentConnect)
			.connect()
			.then(dbh => dbh.begin()
				.then(() => dbh.upsert(this, query))
				.then(() => dbh.commit())
			);
	}
	delete(query) {
		return new Dbi(this.dbConf.url, this.dbConf.writeUser, this.dbConf.persitentConnect)
			.connect()
			.then(dbh => dbh.begin()
				.then(() => dbh.delete(this, query))
				.then(() => dbh.commit())
			);
	}
	onBeforInsert(rec) {
	}
	onAfterInsert(rec) {
	}
	onBeforUpdate(rec, old) {
	}
	onAfterUpdate(rec, old) {
	}
	onBeforeDelete(rec) {
	}
	onAfterDelete(old) {
	}
};

class OrderDeviceRec extends Record {
	table = "order_devices";
	fields = {
		id: {
			type: Number,
			required: true//,
//			primary: true
		},
		createDate: {
			field: "create_date"
			type: Date
		},
		device: {
			feild: "device_id",
			record: DeviceRec,
			foreignField: "id",
			onFeild: "device_id",

			type: Number,
			required: true,
		},
		histories: {
			foreignField: "order_device_id"
			onFeild: "id",
			record: {
				table: "order_device_histories",
				fields: {
					id: {
					},
					action: {
					}
				}
			},
			where: {
				is_acrive: true,
			},
			order: {
				createDate: "desc"
			}
		},

		device: new DeviceRec({
			ref: {
				oneToMany: true,
				foreignField: "id",
				onFeild: "device_id",
				type: Number,
				required: true
			}
		}),
		paymentsStat: new Record({
			ref: {
				oneToMany: false,
				table: "payments_stats",
				foreignField: "id"
				onField: "payments_stat_id"
			},
			feilds: {
				id: {
					type: Number
				},
				name: {
					type: String
				}
			}
		}),
		histories: new OrderDeviceHistoryRec({
			ref: {
				oneToMany: true,
				foreignField: "order_device_id",
				onFeild: "id",
				where: {
					is_acrive: true,
				},
				order: {
					createDate: "desc"
				}
			}
		}),
		tags: new Record({
			ref: {
				oneToMany: true,
				table: "order_device_tags",
				foreignField: "order_device_id",
				onFeild: "id",
				where: {
					is_acrive: true
				}
			}
			record: new Record({
				ref: {
					oneToMany: false,
					table: "tags",
					foreignField: "tag_id",
					onField: "id",
					order: {
						priority: "desc",
						name: "asc"
					}
				},
				fields: {
					id: {
						type: Number,
//						primary: true,
						required: true
					},
					name: {
						type: String,
						required: true
					}
				}
			})
		})
	};
	onBeforInsert(rec) {
		if (!rec.createDate) {
			rec.createDate = new Date();
		}
	}
};
const rec = new OrderDevicesRec({
	dbConf: {
		url: conf.mysql,
		readUser: conf.mysql.readUser,
		writeUser: conf.mysql.writeUser
		persitentConnect: true
	}
});
export default req => rec.execute(JSON.parse(req.query.query));





export default req => {
	const res = [];
	for (const m in JSON.parse(req.query.query)) {
		const f = this[m];
		if (f) {
			res.push(f(query[m]));
		}
	}
	return res;
};
function query(q) {
	new OrderDeviceRec(q.fields);
}
