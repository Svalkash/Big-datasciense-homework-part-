#include "pch.h"
#include "DFA.h"
#include "NFA.h"
#include "RegExpr.h"

using namespace std;

namespace bhw
{
	int DFA::TNnum = 0;

	DFA::DFA(std::ifstream &ifs) : S(), errS(nullptr)
	{
		char c;
		//symb
		ifs.get(c);
		if (c != '[')
			throw exception("wrong_input");
		while (c != ']')
		{
			while (c != '\'')
				ifs.get(c);
			ifs.get(c);
			transes.insert(c);
			ifs.get(c);
			if (c != '\'')
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
			S.insert(new DGNode(ns));
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
			if (DGNode *esp = findNum(es))
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
				char tr;
				int to;
				ifs >> fr;
				do
					ifs.get(c);
				while (c != '\'');
				ifs.get(c);
				tr = c;
				ifs.get(c);
				if (c != '\'')
					throw exception("wrong_trans");
				while (c != ',')
					ifs.get(c);
				ifs >> to;
				if (transes.find(tr) != transes.end())
					addConnect(fr, tr, to);
				ifs.get(c);
				ifs.get(c);
			}
		}
		while (ifs.good() && c != '\n')
			ifs.get(c);
		//groupID
		for_each(S.begin(), S.end(), [&](DGNode *dgn)
		{
			if (find(endS, dgn))
				dgn->grID = 1;
		});
	}

	DFA::DFA(const NFA & na): S(), endS(), transes(na.transes), errS(nullptr)
	{
		//remove epsilon
		transes.erase('\0');
		//other
		std::pair<std::set<int>, bool> p = na.allEps(na.begS);
		DGNode *nn = new DGNode(++TNnum, p.first);
		S.insert(nn);
		begS = nn;
		if (p.second)
			endS.push_back(nn);
		//noRec
		queue<DGNode*> Q;
		Q.push(nn);
		while (Q.size())
		{
			tryAllTranses(Q.front(), na, Q);
			Q.pop();
		}
	}

	DFA::DFA(const std::string & str) : S(), endS(), transes(), errS(nullptr)
	{
		 auto upgrTr = upgradeTree(getTree(str));
		 int endP = dynamic_cast<TLeaf*>(upgrTr.first->right)->Pos;
		 //transes
		 for (auto it : upgrTr.second)
			 if (it.second->type == a)
				 transes.insert(it.second->c);
		 //firstState
		 DGNode *nn = new DGNode(++TNnum, upgrTr.first->Fp);
		 S.insert(nn);
		 begS = nn;
		 if (nn->GNlist.find(endP) != nn->GNlist.end())
			 endS.push_back(nn);
		 //again)
		 queue<DGNode*> Q;
		 Q.push(nn);
		 while (Q.size())
		 {
			 tryAllFollow(Q.front(), upgrTr, Q);
			 Q.pop();
		 }
		 //clearing
		delete upgrTr.first;
	}

	void DFA::renum()
	{
		int in = 1;
		for_each(S.begin(), S.end(), [&in](DGNode *gnp) { gnp->num = in++; });
	}

	DGNode * DFA::findNum(int n) const
	{
		for (auto it : S)
			if (it->num == n)
				return it;
		return nullptr;
	}

	void DFA::addNode()
	{
		DGNode *p = new DGNode;
		S.insert(p);
		renum();
	}

	void DFA::addNode(DGNode *gn)
	{
		bool rf = 0;
		if (findNum(gn->num))
			rf = 1;
		S.insert(gn);
		if (rf)
			renum();
	}

	bool DFA::addConnect(DGNode * from, char tr, DGNode * to)
	{
		if (S.find(from) == S.end() || S.find(to) == S.end())
			throw exception("wrong_connection");
		pair<char, DGNode*> p1 = std::make_pair(tr, to);
		return from->trans.insert(p1).second;
	}

	bool DFA::addConnect(int from, char tr, int to)
	{
		return addConnect(findNum(from), tr, findNum(to));
	}

	DFA::~DFA()
	{
		for_each(S.begin(), S.end(), [](DGNode *gnp) {delete gnp; });
	}

	void DFA::tryAllTranses(DGNode * dgn, const NFA & na, std::queue<DGNode*> &Qr)
	{
		for_each(transes.begin(), transes.end(), [&](char tr)
		{
			bool isEnding = 0;
			set<int> GNset;

			set<GNode*> dest;
			//getting GN set(set!!)
			for_each(dgn->GNlist.begin(), dgn->GNlist.end(), [&](int n)
			{
				for_each(na.findNum(n)->trans.begin(), na.findNum(n)->trans.end(), [&](pair<char, GNode*> tp)
				{
					if (tp.first == tr)
						dest.insert(tp.second);
				});
			});
			//filling
			for_each(dest.begin(), dest.end(), [&](GNode *gnp)
			{
				std::pair<std::set<int>, bool> np = na.allEps(gnp);
				addSets(GNset, np.first);
				if (np.second)
					isEnding = 1;
			});
			//ERROR
			if (GNset.size())
			{
				//finding
				DGNode *stateFound = nullptr;
				for_each(S.begin(), S.end(), [&](DGNode *dgp) { if (dgp->GNlist == GNset) stateFound = dgp; });
				if (stateFound)
					addConnect(dgn, tr, stateFound);
				//creating new
				else
				{
					DGNode *nn = new DGNode(++TNnum, GNset);
					S.insert(nn);
					if (isEnding)
						endS.push_back(nn);
					addConnect(dgn, tr, nn);
					Qr.push(nn);
				}
			}
			else
			{
				if (!errS)
				{
					DGNode *nn = new DGNode(++TNnum);
					S.insert(nn);
					errS = nn;
					for_each(transes.begin(), transes.end(), [&](char etr) { addConnect(nn, etr, nn); });
				}
				addConnect(dgn, tr, errS);
			}
		});
	}

	void DFA::print(std::vector<std::string> &sv) const
	{
		ostringstream ss;
		sv.clear();
		//symbols
		for_each(transes.begin(), transes.end(), [&](char c) { ss << c << ','; });
		sv.push_back(ss.str());
		sv[0].pop_back(); //removing last ','
		//states
		ss.str("");
		for_each(S.begin(), S.end(), [&](DGNode *gnp) { ss << gnp->num << ','; });
		sv.push_back(ss.str());
		sv[1].pop_back(); //removing last ','
		//start
		ss.str("");
		ss << begS->num;
		sv.push_back(ss.str());
		//end, iterations
		ss.str("");
		//ss << endS[0]->num;
		for_each(endS.begin(), endS.end(), [&](DGNode *gnp) { ss << gnp->num << ','; });
		sv.push_back(ss.str());
		sv[3].pop_back(); //removing last ','
		//trans
		sv.push_back(string("")); //preparing place
		for_each(S.begin(), S.end(), [&](DGNode *gnp)
		{
			//out
			for_each(gnp->trans.begin(), gnp->trans.end(), [&](std::pair<char, DGNode*> p)
			{
				ss.str("");
				ss << gnp->num << ',';
				ss << p.first << ':' << p.second->num;
				sv[4] += ss.str();
				sv[4] += ';';
			});
		});
		sv[4].pop_back(); //removing last ';'
	}

	DFA *DFA::minimize()
	{
		//groupfking
		int maxGrNum = 1;
		bool changeFlag = 0;
		do
		{
			changeFlag = 0;
			for (auto it1 = S.begin(); it1 != S.end(); ++it1)
			{
				auto it2 = it1; //need it
				for (it2++; it2 != S.end(); ++it2) //checks it1 == it1... not a problem
					if ((*it1)->grID == (*it2)->grID && !compTrGr(*it1, *it2))
					{
						maxGrNum++;
						//extracting it2 with all its "friends" (definetely after him)
						auto it3 = it2; //need it
						for (it3++; it3 != S.end(); ++it3)
							if ((*it2)->grID == (*it3)->grID && compTrGr(*it2, *it3))
								(*it3)->grID = maxGrNum;
						(*it2)->grID = maxGrNum;
						changeFlag = 1;
						break;
					}
				if (changeFlag)
					break;
			}
		} while (changeFlag);
		//creating and finding "specials" and transes
		DFA *ndfa = new DFA();
		ndfa->transes = transes;
		//nodes
		//need to check if G0 is empty
		int firstGr = 1;
		for (auto it = S.begin(); it != S.end(); ++it)
			if ((*it)->grID == 0)
			{
				firstGr = 0;
				break;
			}
		for (int i = firstGr; i <= maxGrNum; ++i)
			ndfa->S.insert(new DGNode(i));
		//beg
		ndfa->begS = ndfa->findNum(begS->grID);
		//ends
		for (auto eit : endS)
		{
			DGNode *dgp = ndfa->findNum(eit->grID);
			if (!dgp->grID) //using grID: all are 0 at beginning, 1 if it's ending
			{
				ndfa->endS.push_back(dgp);
				dgp->grID = 1;
			}
		}
		//trans
		for (auto it : S)
			if (!ndfa->findNum(it->grID)->trans.size())
				for (auto tit : it->trans)
					ndfa->findNum(it->grID)->trans.insert(make_pair(tit.first, ndfa->findNum(tit.second->grID)));
		//done
		return ndfa;
	}

	bool DFA::compTrGr(const DGNode * n1, const DGNode * n2) const
	{
		for (auto it : transes)
			if (n1->trans.at(it) != n2->trans.at(it))
				return 0;
		return 1;
	}
	
	void DFA::tryAllFollow(DGNode * dgn, const std::pair<TNode*, std::map<int, TLeaf*>>& upgrTr, std::queue<DGNode*>& Qr)
	{
		int endP = dynamic_cast<TLeaf*>(upgrTr.first->right)->Pos;
		for_each(transes.begin(), transes.end(), [&](char tr)
		{
			bool isEnding = 0;
			set<int> GNset;
			//getting GN set(set!!)
			for_each(dgn->GNlist.begin(), dgn->GNlist.end(), [&](int n)
			{
				if (upgrTr.second.at(n)->type == a && upgrTr.second.at(n)->c == tr)
					addSets(GNset, upgrTr.second.at(n)->Follow);
			});
			if (GNset.size())
			{
				//finding
				DGNode *stateFound = nullptr;
				for_each(S.begin(), S.end(), [&](DGNode *dgp) { if (dgp->GNlist == GNset) stateFound = dgp; });
				if (stateFound)
					addConnect(dgn, tr, stateFound);
				//creating new
				else
				{
					DGNode *nn = new DGNode(++TNnum, GNset);
					S.insert(nn);
					if (GNset.find(endP) != GNset.end())
						endS.push_back(nn);
					addConnect(dgn, tr, nn);
					Qr.push(nn);
				}
			}
			//ERROR
			else
			{
				if (!errS)
				{
					DGNode *nn = new DGNode(++TNnum);
					S.insert(nn);
					errS = nn;
					for_each(transes.begin(), transes.end(), [&](char etr) { addConnect(nn, etr, nn); });
				}
				addConnect(dgn, tr, errS);
			}
		});
	}

	std::string DFA::getRE_Kp() const
	{
		vector<vector<vector<string>>> tab(S.size() + 1, vector<vector<string>>(S.size() + 1, vector<string>(S.size() + 1)));
		//k = 0
		for (auto i = S.begin(); i != S.end(); ++i)
			for (auto j = S.begin(); j != S.end(); ++j)
			{
				for (auto t : (*i)->trans)
					if (t.second == (*j))
					{
						if (tab[(*i)->num][(*j)->num][0].length())
							tab[(*i)->num][(*j)->num][0] += '|';
						tab[(*i)->num][(*j)->num][0] += t.first;
					}
				if (!tab[(*i)->num][(*j)->num][0].length() && i != j)
					tab[(*i)->num][(*j)->num][0] = "\\";
			}
		//k = 1..n
		for (unsigned int k = 1; k <= S.size(); ++k)
			for (unsigned int i = 1; i <= S.size(); ++i)
				for (unsigned int j = 1; j <= S.size(); ++j)
				{
					if ((i != k && k != j) && tab[i][j][k - 1] != "\\") //simplify
						tab[i][j][k] = tab[i][j][k - 1];
					if (tab[i][k][k - 1] != "\\" && tab[k][j][k - 1] != "\\") //k-k definetely not \0
					{
						if (tab[i][j][k].length())
							tab[i][j][k] += '|';
#if (POOPA)
						if (i != k) //otherwise, we can have LOOP: r(r)*r
#endif
						{
							tab[i][j][k] += '(';
							tab[i][j][k] += tab[i][k][k - 1];
							tab[i][j][k] += ')';
						}
						if (tab[k][k][k - 1].length())
						{
							tab[i][j][k] += '('; //hope it won't cause too much problems
							tab[i][j][k] += tab[k][k][k - 1];
							tab[i][j][k] += ")*";
						}
#if (POOPA)
						if (k != j) //otherwise, we can have LOOP: r(r)*r
#endif
						{
							tab[i][j][k] += '(';
							tab[i][j][k] += tab[k][j][k - 1];
							tab[i][j][k] += ')';
						}
					}
					else if (!tab[i][j][k].length() && i != j)
						tab[i][j][k] = "\\";
					if (k == S.size() && i == j && !tab[i][j][k].length())
						tab[i][j][k] = "()";
				}
		string ret;
		for (auto it : endS)
		{
			if (ret.length())
				ret += '|';
			ret += tab[begS->num][it->num][S.size()];
		}
		//getting rid of ()
		return buildRE(getTree(ret));
	}

	std::string DFA::getRE_Es() const
	{
		string ret;
		vector<vector<string>> tab(S.size() + 1, vector<string>(S.size() + 1));
		for (auto &i : S)
			for (auto &t : i->trans)
			{
				if (tab[i->num][t.second->num].length())
					tab[i->num][t.second->num] += '|';
				tab[i->num][t.second->num] += t.first;
			}
		set<int> SL; //states left
		for (auto &i : S)
			SL.insert(i->num);
		unsigned int minSize = endS.size() + (!find(endS, begS)); //bool must sum
		while (SL.size() > minSize)
		{
			auto del = SL.begin();
			while (find(endS, findNum(*del)))
				++del;
			//del can be deleted
			delState(SL, tab, del);
		}
		//only beg & ends
		for (auto &dest : SL) //can be optimized... but I'm too lazy
			if (dest != begS->num)
			{
				std::set<int> SL1(SL);
				std::vector<std::vector<std::string>> tab1(tab); //hope it works recursively
				while (SL1.size() > 2)
				{
					auto del1 = SL1.begin();
					while (*del1 == begS->num || *del1 == dest)
						++del1;
					//del can be deleted
					delState(SL1, tab1, del1);
				}
				//only beg&dest
				if (!tab1[begS->num][dest].length())
					continue; //check if it's possible at all
				if (ret.length())
					ret += '|';
				bool smallLoop1 = tab1[begS->num][begS->num].length() > 0;
				bool bigLoop = tab1[dest][begS->num].length() > 0; //forth - already checked
				bool smallLoop2 = tab1[dest][dest].length() > 0;
				if (smallLoop1 || bigLoop) //do we need loop at all?
				{
					ret += '(';
					//loops
					if (smallLoop1)
						ret += tab1[begS->num][begS->num];
					if (smallLoop1 && bigLoop)
						ret += '|';
					if (bigLoop)
					{
						ret += '(';
						ret += tab1[begS->num][dest];
						ret += ')';
						if (smallLoop2)
						{
							ret += '(';
							ret += tab1[dest][dest];
							ret += ")*";
						}
						ret += '(';
						ret += tab1[dest][begS->num];
						ret += ')';
					}
					//done
					ret += ")*";
				}
				ret += '(';
				ret += tab1[begS->num][dest];
				ret += ')';
				if (smallLoop2)
				{
					ret += '(';
					ret += tab1[dest][dest];
					ret += ")*";
				}
			}
			else if (find(endS, begS))
			{
				std::set<int> SL1(SL);
				std::vector<std::vector<std::string>> tab1(tab); //hope it works recursively
				while (SL1.size() > 2)
				{
					auto del1 = SL1.begin();
					while (*del1 == begS->num || *del1 == dest)
						++del1;
					//del can be deleted
					delState(SL1, tab1, del1);
				}
				//only beg=dest
				if (ret.length())
					ret += '|';
				if (!tab1[begS->num][dest].length()) //also smallLoop
				{
					ret += "()";
					continue;
				}
				ret += '(';
				ret += tab1[begS->num][dest];
				ret += ")*";
			}
		//getting rid of ()
		return buildRE(getTree(ret));
	}

	void delState(std::set<int>& SL, std::vector<std::vector<std::string>>& tab, std::set<int>::iterator & del)
	{
		for (auto &fr : SL)
			if (fr != *del && tab[fr][*del].length())
				for (auto &to : SL)
					if (to != *del && tab[*del][to].length())
					{
						if (tab[fr][to].length())
							tab[fr][to] += '|';
						//againlotsof()
						tab[fr][to] += '(';
						tab[fr][to] += tab[fr][*del];
						tab[fr][to] += ')';
						if (tab[*del][*del].length())//cycle?
						{
							tab[fr][to] += '(';
							tab[fr][to] += tab[*del][*del];
							tab[fr][to] += ")*";
						}
						tab[fr][to] += '(';
						tab[fr][to] += tab[*del][to];
						tab[fr][to] += ')';
					}
		SL.erase(del);
	}
}