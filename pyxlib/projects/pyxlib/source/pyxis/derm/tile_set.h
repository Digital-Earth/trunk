#ifndef PYXIS__DERM__TILE_SET_H
#define PYXIS__DERM__TILE_SET_H
/******************************************************************************
tile_set.h

begin		: 2007-08-07
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/index.h"
#include "pyxis/utility/abstract_iterator.h"
#include "pyxis/utility/memory_manager.h"

// standard includes
#include <vector>

/* TODO: Optimizations such as:
- Storing compact indices.
- Auto aggregation.
- Reconsider choice of collection classes for members.
*/

//#define PERFORM_TREE_VALIDATION

/*!
This class encapsulates a set of pyxis tiles, 
and allows serialization to and from file in a compact format
devised by Marc Lepage.
*/
class PYXLIB_DECL PYXTileSet
{
	class Tree
	{
		// TO DO:
		// - Consider changing m_vSubstring to hold actual index digits, and
		// m_vChildNodes to be either empty, or size 7 and sparse.
		// - Consider, with the above change, modifying m_vChildNodes
		// to hold actual nodes rather than pointers.  Splitting a node for insertion would require
		// the new node to be inserted in front so that all the children wouldn't need to be copied.
		class Node : ObjectMemoryUsageCounter<Node>
#ifdef INSTANCE_COUNTING
			, protected InstanceCounter
#endif
		{
			// Substring for node.
			// Each byte within contains 1 set bit, with the exception of
			// the last byte, which can contain more (each corresponding to a 
			// child).
			std::vector<char> m_vSubstring;

			// Vector of child nodes.
			// Offsets are given by multi-bit at end of substring above.
			std::vector<Node *> m_vChildNodes;

		private:

			static bool containsDigit(const unsigned int nSubstringByte, const unsigned int nDigit);

			Node * childNode(const unsigned int nDigit) const;
			void copyChildNodes(const Node& source);
			void clearChildNode(unsigned int nDigit);
			void removeChildNode(unsigned int nDigit);
			void removeChildNodes();

			bool contains(const PYXIndex& index, unsigned int nDigitOffset) const;
			bool intersects(const PYXIndex& index, unsigned int nDigitOffset) const;
			void insert(const PYXIcosIndex& icosIndex, unsigned int nDigitOffset, bool bAggregate);
			void insertWithoutLocalAggregation(const PYXIcosIndex& icosIndex, unsigned int nDigitOffset, bool bAggregateChild);
			void aggregate(const PYXIcosIndex& icosIndex, unsigned int nDigitOffset);

		public:

			/*!
				This iterator must be advanced once
				prior to extracting a value.

				When calling next to move past the last index,
				'end' is not true until the next time through.
			*/
			//! An iterator that iterates root indices.
			class Iterator
			{
				// The node to iterate.
				Node& m_node; 

				// The count of children already visited.
				// Counts the substring as 1 child.
				unsigned int m_nVisitedChildCount;

				// Owned child iterator pointer (or null).
				std::auto_ptr<Node::Iterator> m_spChildNodeIterator;

				// Storage for the current resolution of the index
				// passed into 'next' prior to travelling down children.
				int m_nResolution;

			private:

				//! Disable.
				void operator =(const Iterator&);

			public:

				//! Construct an iterator.
				explicit Iterator(Node& node);

				//! Construct a copy.
				Iterator(const Iterator& source);

				//! Destroy the instance.
				~Iterator();

				//! Iterate past the next index and update 'index' in the process.
				void next(PYXIndex& index);

				/*!
				See if we have covered all the items.

				\return	true if all items have been covered, otherwise false.
				*/
				bool end() const;
			};

			//! Construct an empty node.
			Node();

			//! Construct a copy.
			Node(const Node& source);

			//! Construct by deserializing from stream.
			explicit Node(std::basic_istream<char>& in);

			//! Construct from index.
			Node(const PYXIcosIndex& icosIndex, bool bAggregate, unsigned int nDigitOffset = 0);

			//! Destructor.
			virtual ~Node();

			//! Assign a copy.
			Node& operator =(const Node& source);

			//! Serialize to stream.
			void serialize(std::basic_ostream<char>& out) const;

			//! Set the resolution, relative to the start of the node.
			void setResolution(unsigned char nResolution);

			//! Return true if the tile set tree node intersects the index.
			bool intersects(const PYXIndex& index) const;

			//! Return true if the tile set tree node contains the index.
			bool contains(const PYXIndex& index) const;

			//! Expand the node as necessary to insert the index.
			void insert(const PYXIcosIndex& icosIndex, bool bAggregate);

			/*!
			If bAggregate, truncate.  If it is a descendant, add all of its siblings.
			If !bAggregate, only remove it if found.

			TO DO: Implement.
			*/
			//! Remove the index.
			void remove(const PYXIcosIndex& icosIndex, bool bAggregate);

			//! Return the child node count.
			unsigned int childNodeCount() const;

			//! Return true if the node is empty (no substring).
			bool empty() const;

#ifdef PERFORM_TREE_VALIDATION
			void validate() const;
#endif
		};

		// Vector of roots.
		// TODO: Depending on time and space performance, consider compacting this 
		// like node child arrays, so that all elements aren't created every time.
		// TODO: Consider using boost::ptr_vector to handle deallocations.
		std::vector<Node *> m_vRoots;

	private:

		// Convert the primary resolution to the root index.
		// Converts eg. "A" from ascii to 13.
		static unsigned int getRootIndex(const int nPrimaryResolution);

		// Convert the root index to the primary resolution.
		static unsigned int getPrimaryResolution(const unsigned int nRootIndex);

	public:

		//! An iterator that iterates root indices.
		class Iterator : public PYXAbstractIterator
		{
			// The tree to iterate.
			const Tree& m_tree;

			// Current root index.
			unsigned int m_nRootIndex;

			// The current index.
			PYXIcosIndex m_index;

			// Owned node iterator within the root node.
			std::auto_ptr<Node::Iterator> m_spiNode;

		private:

			//! Disable.
			void operator =(const Iterator&);

			void nextNonEmptyRootNodeIterator();

		public:

			//! Construct an iterator.
			explicit Iterator(const Tree& tree);

			//! Construct a copy.
			Iterator(const Iterator& source);

			//! Destroy the instance.
			~Iterator();

			//! Move to the next item.
			void next();

			/*!
			See if we have covered all the items.

			\return	true if all items have been covered, otherwise false.
			*/
			bool end() const;

			//! Return the current item.  If past end, return the last item.
			const PYXIcosIndex& operator *() const;
		};

		//! Construct an empty tree.
		Tree();

		//! Construct a copy.
		Tree(const Tree& source);

		//! Construct by deserializing from stream.
		explicit Tree(std::basic_istream<char>& in);

		//! Destructor.
		virtual ~Tree();

		//! Assign a copy.
		Tree& operator =(const Tree& source);

		//! Serialize to stream.
		void serialize(std::basic_ostream<char>& out) const;

		//! Return true if the tile set tree intersects the index.
		bool intersects(const PYXIcosIndex& index) const;

		//! Return true if the tile set tree contains the index.
		bool contains(const PYXIcosIndex& index) const;

		//! Expand the tree as necessary to insert the index.
		void insert(const PYXIcosIndex& index, bool bAggregate);

		//! Set the resolution.
		void setResolution(const unsigned char nResolution);

		//! Clear the tree.
		void clear();

		//! Returns true if empty.
		bool empty() const;
	};

	// The tree holding the root indices.
	Tree m_tree;

	// The resolution of cells indicated by all tiles in the set.
	// If zero, it means that the resolution is that of the largest index.
	unsigned char m_nResolution;

public:

	//! An iterator that iterates root indices.
	class Iterator : public PYXAbstractIterator
	{
		// The tree iterator to use.
		Tree::Iterator m_iTree;

	private:

		//! Disable.
		void operator =(const Iterator&);

	public:

		//! Construct an iterator.
		explicit Iterator(const PYXTileSet& set);

		//! Destroy the instance.
		~Iterator();

		//! Move to the next item.
		void next();

		/*!
		See if we have covered all the items.

		\return	true if all items have been covered, otherwise false.
		*/
		bool end() const;

		//! Return the current item.  If past end, return the last item.
		const PYXIcosIndex& operator *() const;
	};

	//! Test method.
	static void test();

	//! Construct an empty tile set.
	//! If no resolution is specified, the resolution is that of the largest index within.
	explicit PYXTileSet(unsigned char nResolution = 0);

	//! Construct by deserializing from stream.
	explicit PYXTileSet(std::basic_istream<char>& in);

	//! Destructor.
	virtual ~PYXTileSet();

	//! Serialize to stream.
	void serialize(std::basic_ostream<char>& out) const;

	//! Return true if the tile set intersects the index.
	bool intersects(const PYXIcosIndex& index) const;

	//! Return true if the tile set contains the index.
	bool contains(const PYXIcosIndex& index) const;

	//! Expand the tile set as necessary to insert the index.
	void insert(const PYXIcosIndex& index, bool bAggregate);

	//! Expand the tile set as necessary to insert each index in the set.
	void insert(const PYXTileSet& set, bool bAggregate);

	//! Set a new resolution.
	void setResolution(const int nResolution);

	//! Get the resolution.
	int resolution() const;

	//! Clear the set.
	void clear();

	//! Returns true if empty.
	bool empty() const;

	//! Returns the number of tiles.
	size_t count() const;
};

//! The equality operator.
PYXLIB_DECL bool operator ==(const PYXTileSet& lhs, const PYXTileSet& rhs);

#endif // guard
