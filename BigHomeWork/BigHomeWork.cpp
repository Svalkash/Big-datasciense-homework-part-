// BigHomeWork.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include "RegExpr.h"
#include "NFA.h"
#include "DFA.h"

using namespace std;
using namespace bhw;

int main()
{
	string ifname, ofname;
	int tasknum;
	cout << "Task number: ";
	cin >> tasknum;
	ifname = "input_";
	ofname = "output_";
	ifname += char(tasknum + 48);
	ofname += char(tasknum + 48);
	ifname += ".txt";
	ofname += ".txt";
	//opening
	ifstream ifs(ifname);
	ofstream ofs(ofname);
	if (!ifs)
	{
		cerr << "cannot_open_file" << endl;
		return 1;
	}
	switch (tasknum)
	{
	case 1:
	{
		string rexp;
		ifs >> rexp;
		TNode *root = getTree(rexp);
		NFA fa(root);
		vector<string> ans;
		fa.print(ans);
		for_each(ans.begin(), ans.end(), [&](string s) { ofs << s << endl; });
		break;
	}
	case 2:
	{
		NFA na(ifs);
		DFA fa(na);
		vector<string> ans;
		fa.print(ans);
		for_each(ans.begin(), ans.end(), [&](string s) { ofs << s << endl; });
		break;
	}
	case 3:
	{
		DFA fa(ifs);
		DFA *ma = fa.minimize();
		vector<string> ans;
		ma->print(ans);
		for_each(ans.begin(), ans.end(), [&](string s) { ofs << s << endl; });
		delete ma;
		break;
	}
	case 4:
	{
		string rexp;
		ifs >> rexp;
		DFA fa(rexp);
		vector<string> ans;
		fa.print(ans);
		for_each(ans.begin(), ans.end(), [&](string s) { ofs << s << endl; });
		break;
	}
	case 5:
	{
		DFA fa(ifs);
		string ans = fa.getRE_Kp();
		ofs << ans << endl;
		break;
	}
	case 6:
	{
		DFA fa(ifs);
		string ans = fa.getRE_Es();
		ofs << ans << endl;
		break;
	}
	default:
		cerr << "wrong_action";
		break;
	}
	ifs.close();
	ofs.close();
	return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
