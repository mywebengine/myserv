#include <string>
#include <fstream>
//#include <iterator>
#include <sstream>

#include "./file.h"
#include "./log.h"

using namespace std;

namespace myfile {

string ReadFile(string& input, string_view url) {
	ifstream r(url.data(), ifstream::in);
	if (!r) {
		return mylog::GetErr("Can`t open file: " + string(url), string_view(__FUNCTION__));
 	}
	string buf((istreambuf_iterator<char>(r)), istreambuf_iterator<char>());
	r.close();
 	input.swap(buf);
	return "";
}

}
