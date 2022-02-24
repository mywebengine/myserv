/*
g++ ./test1.cc -I/home/alex/cc/v8 -I/home/alex/cc/v8/include -lv8_monolith -L/home/alex/cc/v8/out.gn/x64.release.sample/obj/ -DV8_COMPRESS_POINTERS -I./src/certify/include -pthread -std=c++17 -Wall -pedantic -ldl -lboost_system -lboost_thread -lssl -lcrypto -o _test1
*/
 
#include <iostream>
//#include <thread>

#include "./src/config.h"
#include "./src/js/js.h"

int main(int argc, char* argv[]) {
	string err = myjs::Init(argv);
	if (err != "") {
		cout << err;
		return 1;
	}
//	for (auto i : conf.tr_len_) {
//		thread {
//			[&conf]() {
				myconfig::Config conf;
				myjs::Instance js(conf);
				err = js.Init();
				if (err != "") {
					cout << err;
					return 1;
				}
//				myhttp::ServerRun(conf, [&ins](myhttp::Req req) {
					err = js.CreateModule(string("./a.js"));
					if (err != "") {
						cout << err << "\n";
						return 11;
					}
					err = js.RunScript("self.pp(-1)");
					if (err != "") {
						cout << err << "\n";
						return 22;
					}
//					return js.Exec(req);
//				});
				conf.Run();
//			}
//		};
//	}
	return 0;
}
