#pragma once
namespace bhw
{
	class TNode;

	class GNode
	{
	public:
		int num;
		std::multimap<char, GNode*> trans;
		GNode(int num1 = 0) : num(num1), trans() {}
	};

	class NFA
	{
	public:
		static int TNnum;
		//
		std::set<char> transes;
		//
		std::set<GNode*> S;
		GNode *begS;
		std::vector<GNode*> endS;
		//
		NFA(): S(), begS(nullptr), endS() {}
		NFA(TNode *root);
		NFA(std::ifstream &ifs);
		void renum();
		GNode *findNum(int n) const;
		void addNode();
		void addNode(GNode *gn); //requires pre-set number or RENUM
		void addConnect(GNode *from, char tr, GNode *to);
		void addConnect(int from, char tr, int to);
		virtual ~NFA();
		void attach(NFA &au, GNode *abeg, GNode *aend); //KILLS SMALL!!
		//
		void print(std::vector<std::string> &sv) const;
		//
		std::pair<std::set<int>, bool> allEps(GNode *gn) const; //bool shows if there is endings
	};

	std::pair<std::set<int>, bool> &addSetBoolPair(std::pair<std::set<int>, bool>& s1, const std::pair<std::set<int>, bool>& s2);
}