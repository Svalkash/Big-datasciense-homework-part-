#include "pch.h"
#include "NFA.h"
#include "RegExpr.h"

using namespace std;

namespace bhw
{
	int NFA::TNnum = 0;

	NFA::NFA(TNode *root): S(), endS(), transes()
	{
		switch (root->type)
		{
		case e:
		case a:
		{
			GNode *na = new GNode(++TNnum);
			GNode *nb = new GNode(++TNnum);
			na->trans.insert(std::make_pair(dynamic_cast<TLeaf*>(root)->c, nb));
			S.insert(na);
			S.insert(nb);
			begS = na;
			endS.push_back(nb);
			break;
		}
		case o:
		{
			NFA a1(root->left), a2(root->right);
			GNode *nbeg = new GNode(++TNnum);
			GNode *nend = new GNode(++TNnum);
			GNode *na1b = new GNode(a1.begS->num);
			GNode *na1e = new GNode(a1.endS[0]->num);
			GNode *na2b = new GNode(a2.begS->num);
			GNode *na2e = new GNode(a2.endS[0]->num);
			nbeg->trans.insert(std::make_pair('\0', na1b));
			nbeg->trans.insert(std::make_pair('\0', na2b));
			na1e->trans.insert(std::make_pair('\0', nend));
			na2e->trans.insert(std::make_pair('\0', nend));
			S.insert(nbeg);
			S.insert(nend);
			S.insert(na1b);
			S.insert(na1e);
			S.insert(na2b);
			S.insert(na2e);
			begS = nbeg;
			endS.push_back(nend);
			attach(a1, na1b, na1e);
			attach(a2, na2b, na2e);
			break;
		}
		case c:
		{
			NFA a1(root->left);
			//--TNnum;
			NFA a2(root->right);
			GNode *nbeg = new GNode(a1.begS->num);
			GNode *nmid = new GNode(a1.endS[0]->num);
			GNode *nend = new GNode(a2.endS[0]->num);
			S.insert(nbeg);
			S.insert(nend);
			S.insert(nmid);
			begS = nbeg;
			endS.push_back(nend);
			attach(a1, nbeg, nmid);
			attach(a2, nmid, nend);
			break;
		}
		case s:
		{
			NFA a1(root->left);
			GNode *nbeg = new GNode(++TNnum);
			GNode *nend = new GNode(++TNnum);
			GNode *na1b = new GNode(a1.begS->num);
			GNode *na1e = new GNode(a1.endS[0]->num);
			nbeg->trans.insert(std::make_pair('\0', na1b));
			nbeg->trans.insert(std::make_pair('\0', nend));
			na1e->trans.insert(std::make_pair('\0', na1b));
			na1e->trans.insert(std::make_pair('\0', nend));
			S.insert(nbeg);
			S.insert(nend);
			S.insert(na1b);
			S.insert(na1e);
			begS = nbeg;
			endS.push_back(nend);
			attach(a1, na1b, na1e);
			break;
		}
		default:
			throw exception("wrong_enum_lol");
		}
		for_each(S.begin(), S.end(), [&](GNode *gnp)
		{
			for_each(gnp->trans.begin(), gnp->trans.end(), [&](std::pair<char, GNode*> p)
			{
				transes.insert(p.first);
			});
		});
	}

	NFA::NFA(std::ifstream &ifs): S()
	{
		char c;
		//symb
		ifs.get(c);
		if (c != '[')
			throw exception("wrong_input");
		while (c != ']')
		{
			string ttr;
			while (c != '\'')
				ifs.get(c);
			do
			{
				ifs.get(c);
				if (c != '\'')
					ttr += c;
			} while (c != '\'');
			if (ttr == "epsilon")
			{
				ttr.clear();
				ttr += '\0';
			}
			if (ttr.length() == 1)
				transes.insert(ttr[0]);
			else
				throw exception("wrong_trans");
			ifs.get(c);
		}
		do
			ifs.get(c);
		while (c != '\n');
		//states
		ifs.get(c);
		if (c != '[')
			throw exception("wrong_input");
		while (c != ']')
		{
			int ns;
			ifs >> ns;
			S.insert(new GNode(ns));
			c = '\0';
			while (c != ',' && c != ']')
				ifs.get(c);
		}
		do
			ifs.get(c);
		while (c != '\n');
		//start
		int sst;
		ifs >> sst;
		begS = findNum(sst);
		do
			ifs.get(c);
		while (c != '\n');
		//end
		ifs.get(c);
		if (c != '[')
			throw exception("wrong_input");
		while (c != ']')
		{
			int es;
			ifs >> es;
			if (GNode *esp = findNum(es))
				endS.push_back(esp);
			else
				throw exception("wrong_input");
			c = '\0';
			while (c != ',' && c != ']')
				ifs.get(c);
		}
		do
			ifs.get(c);
		while (c != '\n');
		//trans
		ifs.get(c);
		if (c != '[')
			throw exception("wrong_input");
		while (c != ']') //[..], [..]]
		{
			ifs.get(c);
			if (c == '[')
			{
				ifs.get(c); //[1, 'e'], [2,3]]
				if (c != '[')
					throw exception("wrong_input");
				int fr; //1, 'e'], [2,3]]
				string tr;
				int to;
				ifs >> fr;
				do
					ifs.get(c);
				while (c != '\'');
				do
				{
					ifs.get(c);
					if (c != '\'')
						tr += c;
				} while (c != '\'');
				while (c != '[')
					ifs.get(c);
				while (c != ']')
				{
					ifs >> to;
					if (tr == "epsilon")
					{
						tr.clear();
						tr += '\0';
					}
					if (tr.length() == 1)
					{
						if (transes.find(tr[0]) != transes.end())
							addConnect(fr, tr[0], to);
						else
							throw exception("wrong_trans");
					}
					else
						throw exception("wrong_trans");
					ifs.get(c);
					ifs.get(c);
				}
				ifs.get(c);
			}
		}
		while (ifs.good() && c != '\n')
			ifs.get(c);
	}

	void NFA::renum()
	{
		int in = 1;
		for_each(S.begin(), S.end(), [&in](GNode *gnp) { gnp->num = in++; });
	}

	GNode * NFA::findNum(int n) const
	{
		for(auto it: S)
			if (it->num == n)
				return it;
		return nullptr;
	}

	void NFA::addNode()
	{
		GNode *p = new GNode;
		S.insert(p);
		renum();
	}

	void NFA::addNode(GNode *gn)
	{
		bool rf = 0;
		if (findNum(gn->num))
			rf = 1;
		S.insert(gn);
		if (rf)
			renum();
	}

	void NFA::addConnect(GNode * from, char tr, GNode * to)
	{
		if (S.find(from) == S.end() || S.find(to) == S.end())
			throw exception("wrong_connection");
		from->trans.insert(std::make_pair(tr, to));
	}

	void NFA::addConnect(int from, char tr, int to)
	{
		addConnect(findNum(from), tr, findNum(to));
	}

	NFA::~NFA()
	{
		for_each(S.begin(), S.end(), [](GNode *gnp) {delete gnp; });
	}

	void NFA::attach(NFA &au, GNode *abeg, GNode *aend)
	{
		bool rf = 0;
		for (auto np : au.S)
		{
			//renum check
			if (findNum(np->num) && np != au.begS && np != au.endS[0])
				rf = 1;
			//moving transitions FROM s/e
			if (np == au.begS)
				for_each(np->trans.begin(), np->trans.end(), [&](std::pair<char, GNode*> p) { abeg->trans.insert(p); });
			else if (np == au.endS[0])
				for_each(np->trans.begin(), np->trans.end(), [&](std::pair<char, GNode*> p) { aend->trans.insert(p); });
			else
				//moving all except start-end
				S.insert(np);
		}
		//TO s/e
		for (auto np : S)
			for (auto p : np->trans)
				if (p.second == au.begS)
					p.second = abeg;
				else
					if (p.second == au.endS[0])
						p.second = aend;
		//counter
		//TNnum -= 2;
		//renum?
		if (rf)
			renum();
		//killing small nfa nodes
		au.S.clear();
		au.begS = nullptr;
		au.endS.clear();
	}

	void NFA::print(std::vector<std::string> &sv) const
	{
		ostringstream ss;
		sv.clear();
		//symbols
		for_each(transes.begin(), transes.end(), [&](char c)
		{
			if (c == '\0')
				ss << "epsilon" << ',';
			else
				ss << c << ',';
		});
		sv.push_back(ss.str());
		sv[0].pop_back(); //removing last ','
		//states
		ss.str("");
		for_each(S.begin(), S.end(), [&](GNode *gnp) { ss << gnp->num << ','; });
		sv.push_back(ss.str());
		sv[1].pop_back(); //removing last ','
		//start
		ss.str("");
		ss << begS->num;
		sv.push_back(ss.str());
		//end, iterations
		ss.str("");
		//ss << endS[0]->num;
		for_each(endS.begin(), endS.end(), [&](GNode *gnp) { ss << gnp->num << ','; });
		sv.push_back(ss.str());
		sv[3].pop_back(); //removing last ','
		//trans
		sv.push_back(string("")); //preparing place
		for_each(S.begin(), S.end(), [&](GNode *gnp)
		{
			//converting to "multitrans"
			map<char, multiset<GNode*>> tc;
			for_each(gnp->trans.begin(), gnp->trans.end(), [&](std::pair<char, GNode*> p)
			{
				try
				{
					tc.at(p.first).insert(p.second);
				}
				catch (out_of_range)
				{
					multiset<GNode*> nms;
					nms.insert(p.second);
					tc.insert(make_pair(p.first, nms));
				}
			});
			//out
			for_each(tc.begin(), tc.end(), [&](std::pair<char, multiset<GNode*>> p)
			{
				ss.str("");
				ss << gnp->num << ',';
				if (p.first == '\0')
					ss << "epsilon" << ':';
				else
					ss << p.first << ':';
				for_each(p.second.begin(), p.second.end(), [&](GNode *gnp1) { ss << gnp1->num << ','; });
				sv[4] += ss.str();
				sv[4].pop_back(); //removing last ','
				sv[4] += ';';
			});
		});
		sv[4].pop_back(); //removing last ';'
	}


	std::pair<std::set<int>, bool> NFA::allEps(GNode * gn) const
	{
		std::pair<std::set<int>, bool> ret(std::set<int>(), 0);
		//noRec
		queue<GNode*> Q;
		Q.push(gn);
		set<GNode*> worked;
		while (Q.size())
		{
			ret.first.insert(Q.front()->num);
			worked.insert(Q.front());
			if (find(endS, Q.front()))
				ret.second = 1;
			for_each(Q.front()->trans.begin(), Q.front()->trans.end(), [&](std::pair<char, GNode*> p)
			{
				if (p.first == '\0')
					if (worked.find(p.second) == worked.end())
						Q.push(p.second);
			});
			Q.pop();
		}
		
		return ret;
	}

	std::pair<std::set<int>, bool>& addSetBoolPair(std::pair<std::set<int>, bool>& s1, const std::pair<std::set<int>, bool>& s2)
	{
		for_each(s2.first.begin(), s2.first.end(), [&s1](const int el) { s1.first.insert(el); });
		if (s2.second) s1.second = 1;
		return s1;
	}
}
