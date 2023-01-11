#ifndef PYXIS__UTILITY__BOUNDING_CIRCLE_SPATIAL_SET_H
#define PYXIS__UTILITY__BOUNDING_CIRCLE_SPATIAL_SET_H
/******************************************************************************
bounding_circle_spatial_set.h

begin		: 2011-01-22
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/bounding_shape.h"


/*! PYXBoundingCircleSpatialSet<T> class

PYXBoundingCircleSpatialSet implement a sphere tree with value of PYXPointer<T>.

the API:
1. add(BoundingCircle,PYXPointer<T> value) - adds an element into a set.
2. get(BoundingCircle,std::list<PYXPointer<T> values) - to retrieve all elements 
	 intersect an area on earth.
	 
To perform more refined iterations of the tree.
3. getRoot() - return the RootNode of the tree.

PYXBoundingCircleSpatialSet<T>::Node API:
1. hasSingleValue - return true if this node contains a single value
2. getValueCount - return the amount of values
3. getValue - return the single value attached to the node
4. getBoundingCircle - return the current bounding circle
5. getSubNodes - return the list of sub nodes.

TODO: add more smart / convenient iterator
*/
template<class T>
class PYXLIB_DECL PYXBoundingCircleSpatialSet : public PYXObject
{
public:

	//The tree node
	class Node
	{
		friend class PYXBoundingCircleSpatialSet;
		
		//the fractions size between node and sub nodes radius.
		static const int knSubNodeRadiusFraction = 5;
		
		//the amount of nodes before starting to group sub nodes.
		static const int knMaxSubNodes = 10;

	public:
		typedef std::list<Node*> NodeList;

	private:
		int					m_valueCount;
		PYXBoundingCircle	m_circle;
		PYXPointer<T>		m_value;
		NodeList			m_subNodes;
		typename NodeList::iterator	m_firstNoValueNodeIt;

	public:
		Node(const PYXBoundingCircle & circle,const PYXPointer<T> & value)
			:	m_circle(circle), m_value(value), m_valueCount(value?1:0)
		{
			m_firstNoValueNodeIt = m_subNodes.end();
		}

		~Node()
		{
			for(NodeList::iterator it = m_subNodes.begin();it != m_subNodes.end();++it)
			{
				delete *it;
			}
		}

	public:
		bool hasSingleValue() const { return m_value; }
		const PYXPointer<T> & getValue() const { return m_value; }
		int getValueCount() const { return m_valueCount; }

		const PYXBoundingCircle & getBoundingCircle() const { return m_circle; }
		const NodeList & getSubNodes() const { return m_subNodes; }

	protected:
		void add(std::auto_ptr<Node> & newValueNode)
		{
			assert(newValueNode->hasSingleValue() && "adding a-non-value node");
			m_valueCount++;

			Node * subNode = 0;
			double maxRadius = m_circle.getRadius() / knSubNodeRadiusFraction ;

			//if the circle we are adding is small enough. a
			if (newValueNode->m_circle.getRadius() < maxRadius)
			{
				//try to find a good subNode to add this new circle into
				for(NodeList::iterator it = m_firstNoValueNodeIt;it != m_subNodes.end();++it)
				{
					if ((*it)->m_circle.contains(newValueNode->m_circle))
					{
						//we found a group that fits
						(*it)->add(newValueNode);

						//make sure we contain this node also
						m_circle += (*it)->m_circle;
						return;
					}

					//find a group that
					PYXBoundingCircle newCircle = (*it)->m_circle + newValueNode->m_circle;
					if (newCircle.getRadius() < maxRadius)
					{
						subNode = *it;
						maxRadius = newCircle.getRadius();
					}
				}
			}

			//we didn't find any sub node that fit, add a new node.
			if (subNode == 0)
			{
				//this is a value node - push it at to the front.
				m_subNodes.push_front(newValueNode.release());
				subNode = m_subNodes.front();
			}
			else
			{
				//we found a sub node, add value to it.
				subNode->add(newValueNode);
			}
			
			//at this point, the subNode (new or update) has been increased. we need to update the root.
			assert(subNode != 0);
			
			//add the new subNode circle into our bounding circle
			m_circle += subNode->m_circle;

#ifdef DEBUG
			assertNode();
#endif

			//if we got to many nodes. check if we can group them togther
			if (m_subNodes.size() > knMaxSubNodes )
			{
				optimizeNode();
			}
		};

	protected:

		void merge(Node & other)
		{
			//Step 1: add all value nodes from other nodes into this node.
			for(NodeList::iterator valueIt = other.m_subNodes.begin();valueIt != other.m_firstNoValueNodeIt; valueIt++)
			{
				//add it as a value node.
				m_subNodes.push_front(*valueIt);
				
				//add the new subNode circle into our bounding circle
				m_circle += (*valueIt)->m_circle;
			}

			//Step2: add all group nodes
			for(NodeList::iterator groupIt = other.m_firstNoValueNodeIt;groupIt != other.m_subNodes.end(); groupIt++)
			{
				//add it as a group node
				m_firstNoValueNodeIt = m_subNodes.insert(m_firstNoValueNodeIt,*groupIt);

				m_circle += (*m_firstNoValueNodeIt)->m_circle;
			}

			//transfer ownership is completed, we collected all nodes. clear the other list
			other.m_subNodes.clear();

			m_valueCount += other.m_valueCount;

			//Step3: merge the sub groups together

			//if we got to many nodes. check if we can group them together
			if (m_subNodes.size() > knMaxSubNodes )
			{
				optimizeNode();
			}
		}

		void optimizeNode()
		{
			double maxRadius = m_circle.getRadius() / knSubNodeRadiusFraction;

#ifdef DEBUG
			assertNode();
#endif

			//Step 1: increase all groups nodes to contain nodes.

			//go over all value items, see if they can be inserted into a subNode
			for(NodeList::iterator valueIt = m_subNodes.begin();valueIt  != m_firstNoValueNodeIt;)
			{
				//if its a small enough node.. try to group it into one of the groups...
				if ((*valueIt)->m_circle.getRadius() < maxRadius)
				{
					Node * subNode = 0;
					double currentMaxRadius = maxRadius;

					//try to find a good subNode to add this new circle into
					for(NodeList::iterator containingIt = m_firstNoValueNodeIt;containingIt  != m_subNodes.end();++containingIt )
					{
						if ((*containingIt)->m_circle.contains((*valueIt)->m_circle))
						{
							//we found a group that fits
							subNode = *containingIt;
							break;
						}

						//find a group that
						PYXBoundingCircle newCircle = (*containingIt)->m_circle + (*valueIt)->m_circle;
						if (newCircle.getRadius() < currentMaxRadius)
						{
							subNode = *containingIt;
							currentMaxRadius = newCircle.getRadius();
						}
					}

					//create a new group for this node
					if (subNode == 0)
					{
						m_firstNoValueNodeIt = m_subNodes.insert(m_firstNoValueNodeIt, std::auto_ptr<Node>(new Node((*valueIt)->m_circle,0)).release() );
						subNode = *m_firstNoValueNodeIt;
					}

					assert(subNode != 0);
					//move the feature to the sub node.

					NodeList::iterator oldValueIt = valueIt;
					//transfer ownership of valueIt pointer to subNode...
					subNode->add(std::auto_ptr<Node>(*valueIt));

					//make sure we contain it
					m_circle += subNode->m_circle;

					++valueIt;
					m_subNodes.erase(oldValueIt);
				}
				else
				{
					++valueIt;
				}
			}

#ifdef DEBUG
			assertNode();
#endif

			//Step2: increase groups to contain each other
			for(NodeList::iterator it = m_firstNoValueNodeIt;it != m_subNodes.end();++it)
			{
				if ((*it)->m_circle.getRadius() > maxRadius)
				{
					//this group is to big. there is no need to join anything to it right now..
					continue;
				}

				//try to find a good subNode to add this new circle into
				NodeList::iterator containingIt = it;
				++containingIt;
				
				while(containingIt != m_subNodes.end())
				{
					//find a group that could be integrated into this group.
					PYXBoundingCircle newCircle = (*containingIt)->m_circle + (*it)->m_circle;

					if (newCircle.getRadius() < maxRadius)
					{
						(*it)->merge(**containingIt);

						//make sure we contained the merged group
						m_circle += (*it)->m_circle;

						//remove the merged set
						NodeList::iterator oldContainingIt = containingIt;
						++containingIt;
						delete *oldContainingIt;
						m_subNodes.erase(oldContainingIt);
					}
					else
					{
						++containingIt;
					}
				}
			}
			
#ifdef DEBUG
			assertNode();
#endif
		}

		void assertNode()
		{
			int valueCount = 0;
			//Step 1: check all value nodes
			for(NodeList::iterator valueIt = m_subNodes.begin();valueIt != m_firstNoValueNodeIt; valueIt++)
			{
				assert(m_circle.contains((*valueIt)->m_circle));
				assert((*valueIt)->m_valueCount==1);
				assert((*valueIt)->m_value);
				valueCount++;
			}

			//Step2: check all group nodes
			for(NodeList::iterator groupIt = m_firstNoValueNodeIt;groupIt != m_subNodes.end(); groupIt++)
			{
				assert(m_circle.contains((*groupIt)->m_circle));
				assert(!(*groupIt)->m_value);
				valueCount+=(*groupIt)->m_valueCount;
			}

			assert(m_valueCount == valueCount);
		}

	public:
		void get(const PYXBoundingCircle & circle,std::list<PYXPointer<T>> & values)
		{
			for(NodeList::iterator it = m_subNodes.begin();it != m_subNodes.end();++it)
			{
				Node & node = (**it);

				if (circle.intersects(node.m_circle))
				{
					if (node.hasSingleValue())
					{
						values.push_back(node.m_value);
					}
					else
					{
						node.get(circle,values);
					}
				}
			}
		}
	};

protected:
	Node m_root;

public:

	void add(const PYXBoundingCircle & circle,const PYXPointer<T> & value)
	{
		assert(!circle.isEmpty() && "can't add empty circle into the tree");

		m_root.add(std::auto_ptr<Node>(new Node(circle,value)));
	}

	void get(const PYXBoundingCircle & circle,std::list<PYXPointer<T>> & values)
	{
		assert(!circle.isEmpty() && "can't add empty circle into the tree");

		m_root.get(circle,values);
	}

	const Node & getRoot() const { return m_root; }

public:
	PYXBoundingCircleSpatialSet() : m_root(PYXBoundingCircle(),PYXPointer<T>())
	{
	}

	static PYXPointer<PYXBoundingCircleSpatialSet> create()
	{
		return PYXNEW(PYXBoundingCircleSpatialSet);
	}
};


#endif // guard
