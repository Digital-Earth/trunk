#pragma once
#ifndef VIEW_MODEL__SURFACE_TREE_H
#define VIEW_MODEL__SRUFACE_TREE_H
/******************************************************************************
surface_tree.h

begin		: 2009-11-01
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/


template<class T>
class Surface::Tree : public PYXObject
{
public:
	class Node : public PYXObject
	{
		friend class Tree;
	public:
		typedef std::vector<PYXPointer<Node>> NodesVector;

	protected:
		NodesVector         m_nodes;
		Tree &				m_tree;
		Node *				m_parent;
		Surface::Patch::Key m_key;
		PYXPointer<T>		m_data;
		int					m_index;			

		Node(Tree & tree,int index)
			: m_parent(NULL),
			  m_tree(tree),
			  m_index(index)
		{
			m_key = Surface::Patch::Key(index);
		}

		Node(Tree & tree,Node * parent,int index)
			: m_parent(parent),
			  m_tree(tree),
			  m_index(index)
		{
			m_key = Surface::Patch::Key(parent->getKey(),index);
		}

		static PYXPointer<Node> create(Tree & tree,int index)
		{
			return PYXNEW(Node,tree,index);
		}

		static PYXPointer<Node> create(Tree & tree,Node * parent,int index)
		{
			return PYXNEW(Node,tree,parent,index);
		}

		void createNodes()
		{
			for(int i=0;i<9;i++)
			{
				m_nodes.push_back(Node::create(m_tree,this,i));
			}
		}

	public:
		const Surface::Patch::Key & getKey() const { return m_key; }

		void divide()
		{
			if (!isDivided())
			{
				createNodes();
			}
		}

		void unify()
		{
			m_nodes.clear();
		}

		bool isDivided() const { return m_nodes.size() != 0; }

		PYXPointer<Node> getSubNode(int index)
		{
			if (index >= 0 && index < (int)(m_nodes.size()))
			{
				return m_nodes[index];
			}
			else
			{
				return PYXPointer<Node>();
			}
		}

		PYXPointer<const Node> getSubNode(int index) const
		{
			if (index >= 0 && index < (int)(m_nodes.size()))
			{
				return m_nodes[index];
			}
			else
			{
				return PYXPointer<Node>();
			}
		}

		PYXPointer<Node> getParent()
		{
			return m_parent;
		}

		PYXPointer<const Node> getParent() const
		{
			return m_parent;
		}

		Node * getParentPtr() const
		{
			return m_parent;
		}

		Node * getSubNodePtr(int index) const
		{
			return m_nodes[index].get();
		}

		Tree & getTree()
		{
			return m_tree;
		}

		const Tree & getTree() const
		{
			return m_tree;
		}

		PYXPointer<T> getData()
		{
			return m_data;
		}

		PYXPointer<const T> getData() const
		{
			return m_data;
		}

		bool hasData() const
		{
			return m_data;
		}

		void setData(const PYXPointer<T> & data)
		{
			m_data = data;
		}

		bool allSubNodesHasData() const
		{
			if (!isDivided())
			{
				return false;
			}

			for(int i=0;i<9;i++)
			{
				if (!m_nodes[i]->hasData())
				{
					return false;
				}
			}

			return false;
		}
	};
protected:
	typedef typename Node::NodesVector NodesVector;
	NodesVector m_nodes;

	void createNodes()
	{
		for(int i=0;i<90;i++)
		{
			m_nodes.push_back(Node::create(*this,i));
		}
	}

public:
	Tree()
	{
		createNodes();
	}

	typename NodesVector::iterator begin() { return m_nodes.begin(); }
	typename NodesVector::iterator end() { return m_nodes.end(); }

	PYXPointer<Node> createNode(const Surface::Patch::Key & key)
	{
		PYXPointer<Node> node;
		if (key.getResolution() < 0)
		{
			return node;
		}	
		node = m_nodes[key.getPatchIndex(0)];
		int res = 1;

		while(node && res <= key.getResolution())
		{
			//Activly divide nodes if needed
			if (!node->isDivided())
			{
				node->divide();
			}				
			node = node->getSubNode(key.getPatchIndex(res));

			res++;
		}

		return node;
	}

	PYXPointer<Node> getNode(const Surface::Patch::Key & key)
	{
		PYXPointer<Node> node;
		if (key.getResolution() < 0)
		{
			return node;
		}	
		node = m_nodes[key.getPatchIndex(0)];
		int res = 1;

		while(node && res <= key.getResolution())
		{
			if (node->isDivided())
			{
				node = node->getSubNode(key.getPatchIndex(res));
			}
			else
			{
				node.reset();
			}
			res++;
		}

		return node;
	}

	PYXPointer<const Node> getNode(const Surface::Patch::Key & key) const
	{
		PYXPointer<const Node> node;
		if (key.getResolution() < 0)
		{
			return node;
		}	
		node = m_nodes[key.getPatchIndex(0)];
		int res = 1;

		while(node && res <= key.getResolution())
		{
			if (node->isDivided())
			{
				node = node->getSubNode(key.getPatchIndex(res));
			}
			else
			{
				node.reset();
			}
			res++;
		}

		return node;
	}

};

#endif