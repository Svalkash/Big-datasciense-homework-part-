#pragma once

namespace bhw
{
	enum NType { e, a, o, c, s, x };

	class TNode
	{
	public:
		//too lazy to protect left-right
		NType type;
		TNode *left, *right;
		bool N;
		std::set<int> Fp, Lp;
		TNode(NType t1 = e, TNode *l1 = nullptr, TNode *r1 = nullptr) :type(t1), left(l1), right(r1), N(false), Fp(), Lp() {}
		virtual ~TNode() { delete left; delete right; }
	};

	class TLeaf : public TNode
	{
	public:
		char c;
		int Pos;
		std::set<int> Follow;
		TLeaf(NType t1 = e, TNode *l1 = nullptr, TNode *r1 = nullptr, char c1 = '\0') :TNode(t1, l1, r1), c(c1), Follow() {}
		virtual ~TLeaf() { delete left; delete right; }
	};

	TNode *getTree(const std::string &str);
	std::string buildRE(TNode *root);
	std::pair<TNode*, std::map<int, TLeaf*>> upgradeTree(TNode *oldRoot);
	void setPos(TNode *root, std::map<int, TLeaf*> &pa);
	void setNfl(TNode *root);
	void setFollows(TNode *root, const std::map<int, TLeaf*> &pa);
}