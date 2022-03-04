#pragma once

#define POOPA 1

namespace bhw
{
	class TNode;
	class TLeaf;
	class GNode;
	class NFA;

	class DGNode
	{
	public:
		int num;
		std::map<char, DGNode*> trans;
		std::set<int> GNlist;
		DGNode(int num1 = 0) : num(num1), trans(), GNlist(), grID(0) {}
		DGNode(int num1, std::set<int> ngn) : num(num1), trans(), GNlist(ngn), grID(0) {}
		int grID;
	};

	class DFA
	{
	public:
		static int TNnum;
		//
		std::set<DGNode*> S;
		DGNode *begS;
		std::vector<DGNode*> endS;
		std::set<char> transes;
		DGNode *errS;
		//
		DFA() : S(), begS(nullptr), endS() {}
		DFA(std::ifstream &ifs);
		DFA(const NFA &na);
		DFA(const std::string &str);
		void renum();
		DGNode *findNum(int n) const;
		void addNode();
		void addNode(DGNode *gn); //requires pre-set number or RENUM
		bool addConnect(DGNode *from, char tr, DGNode *to);
		bool addConnect(int from, char tr, int to);
		virtual ~DFA();
		//
		void tryAllTranses(DGNode *dgn, const NFA &na, std::queue<DGNode*> &Qr);
		//
		void print(std::vector<std::string> &sv) const;
		//
		DFA *minimize();
		bool compTrGr(const DGNode *n1, const DGNode *n2) const;
		//
		void tryAllFollow(DGNode *dgn, const std::pair<TNode*, std::map<int, TLeaf*>> &upgrTr, std::queue<DGNode*> &Qr);
		//
		std::string getRE_Kp() const;
		std::string getRE_Es() const;
	};

	void delState(std::set<int> &SL, std::vector<std::vector<std::string>> &tab, std::set<int>::iterator &del);
}