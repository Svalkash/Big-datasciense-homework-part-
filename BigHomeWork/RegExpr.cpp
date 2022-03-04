#include "pch.h"
#include "RegExpr.h"

using namespace std;
namespace bhw
{
	TNode *getTree(const std::string &str)
	{
		TNode *retR, *retL;
		/*
		1. |
		2. .
		3. *
		4. ()
		*/
		if (str.length() > 1)
		{
			int brcount = 0;
			int lastO = -1, lastC = -1, lastS = -1, lastRbr = -1, firstLbr = -1;
			for (int i = str.length() - 1; i >= 0; --i)
				switch (str[i])
				{
				case ')':
					/*
					if (lastRbr < 0)
						lastRbr = i;
					*/
					brcount++;
					break;
				case '(':
					//firstLbr = i;
					brcount--;
					if (lastC < 0 && i > 0 && str[i - 1] != '|'/*impossible*/ && str[i - 1] != '(' && !brcount)
						lastC = i; //concat = RIGHTpos
					break;
				case '|': //if not inside br
					if (lastO < 0 && !brcount)
						lastO = i;
					break;
				case '*': //if not inside br
					if (lastS < 0 && !brcount)
						lastS = i;
					break;
				default:
					/*
					| will be eliminated
					possible:
					...)a
					...*a
					a(
					a.b* included
					ble, right
					)*
					*/
					if (lastC < 0 && i > 0 && str[i - 1] != '|'/*impossible*/ && str[i - 1] != '(' && !brcount)
						lastC = i; //concat = RIGHTpos
					break;
				}
			if (lastO >= 0)
			{
				retL = getTree(str.substr(0, lastO)); //lasO symbols up to last-1
				retR = getTree(str.substr(lastO + 1));
				return new TNode(o, retL, retR);
			}
			if (lastC >= 0)
			{
				retL = getTree(str.substr(0, lastC));
				retR = getTree(str.substr(lastC));
				return new TNode(c, retL, retR);
			}
			if (lastS >= 0)
			{
				retL = getTree(str.substr(0, lastS));
				retR = nullptr;
				return new TNode(s, retL, retR);
			}
			//Don't check Rbr-Lbr: if none of previous and >3 => definetely Brackets... remove them
			return getTree(str.substr(1, str.length() - 2));
		}
		else
		{
			//symbol
			if (str.length())
				return new TLeaf(a, nullptr, nullptr, str[0]);
			//epsilon
			else
				return new TLeaf(e);
		}
	}

	std::string buildRE(TNode * root)
	{
		string str;
		switch (root->type)
		{
		case e:
			str = "()";
			break;
		case a:
			str += dynamic_cast<TLeaf*>(root)->c;
			break;
		case o:
			str += buildRE(root->left);
			str += '|';
			str += buildRE(root->right);
			break;
		case c:
			if (root->left->type == o)
			{
				str += '(';
				str += buildRE(root->left);
				str += ')';
			}
			else
				str += buildRE(root->left);
			//no symbol
			if (root->right->type == o)
			{
				str += '(';
				str += buildRE(root->right);
				str += ')';
			}
			else
				str += buildRE(root->right);
			break;
		case s:
			if (root->left->type == o || root->left->type == c)
			{
				str += '(';
				str += buildRE(root->left);
				str += ')';
			}
			else
				str += buildRE(root->left);
			str += '*';
			break;
		}
		return str;
	}

	std::pair<TNode*, std::map<int, TLeaf*>> upgradeTree(TNode * oldRoot)
	{
		//adding #
		std::pair<TNode*, std::map<int, TLeaf*>> ret;
		ret.first = new TNode(c, oldRoot, new TLeaf(x, nullptr, nullptr));
		//rest
		setPos(ret.first, ret.second);
		setNfl(ret.first);
		setFollows(ret.first, ret.second);
		return ret;
	}

	void setPos(TNode * root, map<int, TLeaf*> &pa) //can be launched ONCE!!!!111!1!1!11
	{
		static int posNum = 0;
		switch (root->type)
		{
		case e:
			break;
		case a:
		case x:
			dynamic_cast<TLeaf*>(root)->Pos = ++posNum;
			pa.insert(make_pair(posNum, dynamic_cast<TLeaf*>(root)));
			break;
		default: //o,s,c
			if (root->left)
				setPos(root->left, pa);
			if (root->right)
				setPos(root->right, pa);
			break;
		}
	}

	void setNfl(TNode * root)
	{
		if (root->left)
			setNfl(root->left);
		if (root->right)
			setNfl(root->right);
		switch (root->type)
		{
		case e:
			root->N = true;
			break;
		case a:
		case x:
			root->N = false;
			root->Fp.insert(dynamic_cast<TLeaf*>(root)->Pos);
			root->Lp.insert(dynamic_cast<TLeaf*>(root)->Pos);
			break;
		case o:
			root->N = root->left->N || root->right->N;
			root->Fp = root->left->Fp;
			addSets(root->Fp, root->right->Fp);
			root->Lp = root->right->Lp;
			addSets(root->Lp, root->left->Lp);
			break;
		case c:
			root->N = root->left->N && root->right->N;
			root->Fp = root->left->Fp;
			if (root->left->N)
				addSets(root->Fp, root->right->Fp);
			root->Lp = root->right->Lp;
			if (root->right->N)
				addSets(root->Lp, root->left->Lp);
			break;
		case s:
			root->N = true;
			root->Fp = root->left->Fp;
			root->Lp = root->left->Lp;
		}
	}

	void setFollows(TNode * root, const map<int, TLeaf*> &pa)
	{
		if (root->left)
			setFollows(root->left, pa);
		if (root->right)
			setFollows(root->right, pa);
		switch (root->type)
		{
		case c:
			for (auto it : root->left->Lp)
				addSets(pa.at(it)->Follow, root->right->Fp);
			break;
		case s:
			for (auto it : root->left->Lp)
				addSets(pa.at(it)->Follow, root->left->Fp);
			break;
		default:
			break;
		}
	}
}