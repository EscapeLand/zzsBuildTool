#include "stdafx.h"
#include "FileOperation.hpp"
#define pause(x) system("pause"); return (x);

using namespace std;

int exec(initializer_list<string> arglist);
inline char prompt(const char s[]) { cout << s << endl; char c; cin >> c; return c; }
void jar(string path, string filename, string mainclass, set<string> lib);
bool getOps(const int argc, char* argv[]);

static mutex mtx;
static atomic_int cnt;
bool multithread = true;
bool silent = false;
bool verbose = false;
static string bin = "bin";

int main(int argc, char* argv[]) try{
	if (!getOps(argc, argv)) {
		pause(0);
	}
	Directory src("src");
	set<string> dirs;
	src.getCurrentSubDir(dirs);
	size_t n = dirs.size();

	cout << "找到" << n << "个工程：" << endl;
	for (string i : dirs) cout << i << endl;
	cout << "\n你设置的二进制目录为：\n";
	for (string i : dirs) cout << "src\\" << i << "\\" << bin <<  endl;

	cout << "\n\n";
	system("pause");
	vector<tuple <string, string, string, set<string>>> buildinfo;

	for (string i: dirs) {
		system("cls");
		Directory proj = src.subDir(i);
		set<string> p1, p2;
		cout << i << endl;
		cout << "检测源文件目录和生成文件目录是否匹配...." << endl;
		proj.getPackage_byBinary(p1, "^src\\\\" + i + "\\\\" + bin + "\\\\?");
		proj.getPackage_bySource(p2, "^src\\\\" + i + "\\\\?");
		if (p1.size() == p2.size()) cout << "匹配." << endl;
		else if (p1.size() > p2.size()) {
			cerr << "注意，源文件和二进制文件数目不匹配，这可能正常但也可能是构建不完全，已忽略此项. " << endl;
		}
		else {
			cout << i + " 未构建完全，" + "将所有源文件编译后重试." << endl;
			cout << endl << i + "构建失败." << endl << endl;
			system("pause");
			continue;
		}
		p2.clear(); 
		proj.subDir(bin).getPackage_byBinary(p2, "^src\\\\" + i + "\\\\" + bin + "\\\\");
		cout << "输入Main Class，含包名，已为您列出可能项：" << endl;
		for (string i : p2) {
			cout << i << endl;
		}
		cout << endl;

		mainclass:
		string tmp;
		cin >> tmp;
		if (!regex_search(tmp, regex("^[a-zA-Z0-9]+(\\.[a-zA-Z0-9]+)*$"))) {
			cerr << "主类名非法：" << tmp << endl;
			cout << endl << i + "构建失败." << endl << endl;
			system("pause");
			continue;
		}
		else {
			if (_access(("src\\" + i + "\\" + bin + "\\" + Directory::package2dir(tmp)).c_str(), 0) != 0) {
				cout << "该类不存在，请检查当前目录下是否存在该文件结构." << endl;
				if (prompt("要重新输入嘛？Y/n") == 'Y') goto mainclass;
				else cout << endl << i + "构建失败." << endl << endl;
				system("pause");
				continue;
			}
		}
		
		set<string> lib;
		if (prompt("要选择依赖库么？Y/n") == 'Y') {
			cout << "\nlib目录下的jar包如下，键入序号选择依赖：\n\n";
			Directory libdir("lib");
			vector<string> files;
			libdir.findAllExt(".jar", files);
			for (size_t i = 0; i < files.size(); i++) {
				cout << i + 1 << ". " << files[i] << endl;	
			}
			cout << "0. 结束（要是不最后选 0 你懂得会发生什么）\n\n";
			int index;
			for (cin >> index; index > 0; cin >> index) {
				lib.insert(files[index - 1]);
			}
			if (index < 0) {
				cout << "what? 你输啥了？？？既然这样，就 continue 罢(￣▽￣)\"\n\n";
			}
		}

		buildinfo.emplace_back(make_tuple(i, "src\\" + i + "\\" + bin, tmp, lib));
	}

	system("cls");
	cnt = (int)buildinfo.size();
	mtx.lock();
	for (auto [proj, bin, mainclass, lib] : buildinfo) {
		thread t(jar, bin, proj + ".jar", mainclass, lib);
		t.detach();
		cout << "thread for " << proj << " is started, id: " << t.get_id() << endl;
	}
	mtx.unlock();

	while (cnt > 0) this_thread::yield();
	pause(0);
}
catch (exception ex) {
	cerr << ex.what() << endl;
	system("pause");
	return -1;
}

int exec(initializer_list<string> arglist) {
	assert(arglist.size() > 0);
	string cmd = *arglist.begin();

	for (auto p = arglist.begin() + 1; p != arglist.end(); p++) {
		cmd.append(" ");
		cmd.append(*p);
	}

	return system(cmd.c_str());
}

/*
 * @param path						binary path
 * @param filename					jar name
 * @param mainclass					as name
 * @param lib						list of library jar
*/
void jar(string path, string filename, string mainclass, set<string> lib) try{
	const string MANIFEST = "Manifest-Version: 1.0\n"
		"Created-By: " + PROJECT + "\nMain-Class: " + mainclass + "\n";

	fstream fso(path + "\\MANIFEST.MF", ios::out);
	if (fso.fail()) {
		cnt--;
		return;
	}

	fso << MANIFEST;
	if (!lib.empty()) {
		string classpath = "Class-Path: \n ";
		for (string i : lib) {
			if (i.length() > 58) i.insert(58, "\n ");
			classpath.append(i + '\n');
		}

		fso << classpath;
	}

	fso.close();

	for (string i : lib) {
		if (exec({ "copy", i, path }) != 0) {
			cnt--;
			return;
		}
	}
	
	string filelist;
	set<string> files;
	Directory proj(path);
	proj.getCurrentStructure(files);
	for (string i : files) filelist.append(" \"").append(i + '"');

	if (exec({ "jar", "cfm", path + "\\" + filename, path + "\\MANIFEST.MF", filelist }) != 0) {			//jar cvfm....
		cnt--;
		return;
	}

	mtx.lock();
	cout << filename + " is packed. " << endl;
	mtx.unlock();
	cnt--;
}
catch (exception ex) {
	cnt--;
}

bool getOps(const int argc, char* argv[]) {
	static const string help = "usage: BuildTool [options] [binary directory]\n"
		"\t-b (path): set your binary directory, default dir is \"bin\"\n"
		"\t-d: multi-thread enabled (by default)\n"
		"\t-s: silent mode, maybe more efficency\n"
		"\t-t: multi-thread disabled (take the last when with -d)\n"
		"\t-v: verbose mode, for trace and debug (take the last when with -s)\n"
		"\t--version: print version information";

	vector<string> ops(argc);
	transform(argv, argv + argc, ops.begin(), [](const char* x) -> const char* {return x; });

	for (int i = 1; i < argc; i++) {
		if (ops[i] == "-d") multithread = true;
		else if (ops[i] == "-j") multithread = false;
		else if (ops[i] == "-h") {
			cout << help;
			return false;
		}
		else if (ops[i] == "-b") {
			if (_access(ops[++i].c_str(), 0) == 0) {
				bin = ops[i];
			}
		}
		else if (ops[i] == "-s") {
			verbose = false;
			silent = true;
		}
		else if (ops[i] == "-v") {
			silent = false;
			verbose = true;
		}
		else if (ops[i] == "--version") {
			cout << PROJECT << endl;
			cout << VERSION << endl;
			return false;
		}
		else {
			throw runtime_error("无法识别的选项：" + ops[i]);
		}
	}

	return true;
}