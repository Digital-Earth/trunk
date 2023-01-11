#if !defined(PYXIS__GLOBE__TREE__SUBTREE_MAP)
#define PYXIS__GLOBE__TREE__SUBTREE_MAP

#include "pyxis/empty.hpp"
#include "pyxis/functor_interface.hpp"
#include "pyxis/globe/tree/index.hpp"
#include <boost/compressed_pair.hpp>
#include <boost/utility/value_init.hpp>
#include <stack>

namespace Pyxis
{
	namespace Globe
	{
		namespace Tree
		{
			// A map of subtree to value.
			// A subtree is a tree within the tree that is defined by a single root index.
			// Value requirements:
			//	-	Empty constructor.
			//	-	Equality (and inequality) operator; if values compare as equal,
			//		they are candidates for storage optimization.
			//	-	If it is a singleton type (i.e. all instances are equivalent),
			//		should specialize IsSingleton to take advantage of optimizations.
			template < typename Value = Empty > class SubtreeMap;
		}
	}
}

template < typename Value >
class Pyxis::Globe::Tree::SubtreeMap : public virtual Pointee
{
	BOOST_STATIC_CONSTANT(size_t, valueSize = IsSingleton< Value >::value ? 0 : sizeof(Value));

	// A pointer to an array of bytes allocated via "malloc"/"realloc".
	/*
	The byte array pointer can be null (an empty node) or a pointer to 
	a byte array in the following format:
	- 	Header (1 byte)
		-	High bit is 1 for leaf, 0 for branch.
		-	Remainder indicates byte count of edge.
	-	Edge (>= 0 bytes)
		-	Stores an array of steps; the byte count is given in header.
		-	Note that the level 0 root step 0 is included, which allows for a simple
			compression strategy (0v -> v) for serialization.
	-	Vertex (>= 0 bytes)
		-	If leaf:
			-	Embedded Value instance, or nothing if Value is Empty
		-	Else (branch):
			-	Byte indicating child count
			-	Embedded Node array
	*/
	class Node
	{
		BOOST_STATIC_CONSTANT(char unsigned, headerIsLeafMask = (1 << (CHAR_BIT - 1)));
		BOOST_STATIC_CONSTANT(char unsigned, headerEdgeCountMask = (char unsigned)(~headerIsLeafMask));

		// The array of bytes.
		// Structured: header (1 byte) + edge + vertex.
		char unsigned * array;

	public:
		
		// Iterates through the steps in the node edge.
		class StepIterator :
		public std::iterator< std::forward_iterator_tag, Step >
		{
			// A pointer to a byte in the node edge.
			char unsigned const * current;
			
			// A pointer to the byte past the node edge.
			char unsigned const * end;

		public:

			bool operator ==(StepIterator const & that) const
			{
				return this == &that || (
					current == that.current && end == that.end);
			}

			bool operator !=(StepIterator const & that) const
			{
				return !(*this == that);
			}

			Step operator *() const
			{
				assert(current);
				return *current;
			}

			Step * operator->() const
			{
				assert(current);
				return current;
			}

			StepIterator & operator ++()
			{
				assert(current);
				assert(current < end);
				if (end <= ++current)
				{
					current = end = 0;
				}
				return *this;
			}

			// Constructs a step iterator in the "begin" state,
			// or the "end" state if the edge has size 0.
			explicit StepIterator(Node const & node) :
			current(), end()
			{
				if (!node.getIsEmpty())
				{
					char const edgeSize = node.getEdgeSize();
					if (edgeSize)
					{
						current = node.getEdge();
						end = node.getEdge() + edgeSize;
					}
				}
			}

			// Constructs a step iterator in the "end" state.
			explicit StepIterator() : current(), end() {}
		};

		// An iterator over [non-empty] children of a node.
		class ChildIterator :
		public virtual Pointee
		{
			Node const * begin;
			Node const * end;
			Node const * current;

			void advance()
			{
				assert(current);
				assert(current < end);

				// Find next non-empty child (or end).
				do
				{
					if (++current == end)
					{
						// Finish.
						current = end = 0;
						break;
					}
				} while (current->getIsEmpty());
			}

			void start()
			{
				assert(current);
				assert(end);
				assert(current < end);
				assert((end - current) <= boost::integer_traits< Step >::const_max);

				if (current->getIsEmpty())
				{
					advance();
				}
			}

		public:

			// Doesn't check the index.
			bool operator ==(ChildIterator const & that) const
			{
				return (&that == this) || (
					that.current == current &&
					that.end == end);
			}

			bool operator !=(ChildIterator const & that) const
			{
				return !(*this == that);
			}

			Node const & operator *() const
			{
				assert(current);
				return *current;
			}

			Node const * operator->() const
			{
				assert(current);
				return current;
			}

			ChildIterator & operator ++()
			{
				advance();
				return *this;
			}
			
			Step getOffset() const
			{
				assert(current < end);
				return current - begin;
			}

			template < typename InputIterator >
			explicit ChildIterator(InputIterator begin, InputIterator end) :
			begin(), end(), current()
			{
				assert(begin <= end);
				if (begin < end)
				{
					this->begin = begin;
					this->end = end;
					this->current = this->begin;
					
					start();
				}
			}
			
			explicit ChildIterator(Node const & node) :
			begin(), end(), current()
			{
				// Get child begin and end from node,
				// set current and end, and call start().
				if (node.getIsBranch())
				{
					this->begin = node.getBranchChildren();
					this->end = this->begin + node.getBranchChildCount();
					this->current = this->begin;

					start();
				}
			}

			// Constructs a child iterator in the end state.
			explicit ChildIterator() :
			begin(), end(), current()
			{}
		};
		
		bool getIsEmpty() const
		{
			return !array;
		}

		bool getIsLeaf() const
		{
			return array && (*array & headerIsLeafMask);
		}

		bool getIsBranch() const
		{
			return array && !(*array & headerIsLeafMask);
		}

	private: // Statics

		static size_t getEdgeOffset()
		{
			// Allow 1 byte to hold header.
			return 1;
		}

		// Offset of children from vertex offset.
		static size_t getVertexChildrenOffset()
		{
			// Allow 1 byte to hold child count.
			return 1;
		}
		
		static size_t computeBranchVertexSize(char const childCount)
		{
			return getVertexChildrenOffset() + (childCount * sizeof(Node));
		}

	private: // Functions that assert that this isn't empty.

		char unsigned const * getEdge() const
		{
			assert(!getIsEmpty());
			return array + getEdgeOffset();
		}

		char getEdgeSize() const
		{
			assert(!getIsEmpty());
			return (*array & headerEdgeCountMask);
		}
		
		size_t getVertexOffset() const
		{
			assert(!getIsEmpty());
			return getEdgeOffset() + getEdgeSize();
		}
		
		size_t getVertexSize() const
		{
			assert(!getIsEmpty());
			return getIsLeaf() ? valueSize : computeBranchVertexSize(getBranchChildCount());
		}

	private: // Functions that assert that this is a leaf.

		// Assumes that the memory has already been allocated.
		void constructLeafVertex(Value const value = Value())
		{
			assert(getIsLeaf());
			if (valueSize)
			{
				new (array + getVertexOffset()) Value(value);
			}
		}
		
		// Assumes that the value has already been created with placement new.
		void destructLeafVertex()
		{
			assert(getIsLeaf());
			if (valueSize)
			{
				((Value *)(array + getVertexOffset()))->~Value();
			}
		}

		void setLeafValue(Value value)
		{
			if (valueSize)
			{
				*(Value *)(array + getVertexOffset()) = value;
			}
		}

		Value getLeafValue() const
		{
			assert(getIsLeaf());
			return valueSize ? *(Value const *)(array + getVertexOffset()) : Value();
		}

	private: // Functions that assert that this is a branch.
	
		// Constructs with empty child nodes.
		void constructBranchVertex(char childCount)
		{
			assert(getIsBranch());
			array[getVertexOffset()] = childCount;
			for (Node * child = getBranchChildren();
				childCount; --childCount, ++child)
			{
				new (child) Node();
			}
		}
		
		// Copies children (and descendants).
		void constructBranchVertex(char childCount, Node const childrenToCopy[])
		{
			assert(getIsBranch());
			array[getVertexOffset()] = childCount;
			for (Node * child = getBranchChildren();
				childCount; --childCount, ++child, ++childrenToCopy)
			{
				new (child) Node(*childrenToCopy);
			}
		}
		
		void destructBranchVertex()
		{
			assert(getIsBranch());
			char childCount = getBranchChildCount();
			for (Node * child = getBranchChildren();
				childCount; --childCount, ++child)
			{
				child->~Node();
			}
		}

		// Swap children of two branches that have the same branch count.
		void swapBranchChildren(Node & branch)
		{
			assert(getIsBranch());
			assert(branch.getIsBranch());
			assert(getBranchChildCount() == branch.getBranchChildCount());

			char childCount = getBranchChildCount();
			while (childCount)
			{
				--childCount; // Do this in a separate statement to avoid undefined behaviour.
				getBranchChild(childCount).swap(branch.getBranchChild(childCount));
			}
		}

		char getBranchChildCount() const
		{
			assert(getIsBranch());
			char unsigned const childCount = array[getVertexOffset()];
			assert(1 < childCount &&
				"If child count is 0, should be a leaf; if 1, should be part of the edge.");
			assert(childCount <= (char unsigned const)boost::integer_traits< char >::const_max);
			return childCount;
		}

		size_t getBranchChildrenOffset() const
		{
			assert(getIsBranch());
			return getVertexOffset() + getVertexChildrenOffset();
		}
		
		Node * getBranchChildren()
		{
			assert(getIsBranch());
			return (Node *)(array + getBranchChildrenOffset());
		}
		
		Node const * getBranchChildren() const
		{
			assert(getIsBranch());
			return (Node const *)(array + getBranchChildrenOffset());
		}
		
		Node & getBranchChild(Step const childOffset)
		{
			assert(getIsBranch());
			assert(childOffset < getBranchChildCount());
			return getBranchChildren()[childOffset];
		}
		
		Node const & getBranchChild(Step const childOffset) const
		{
			assert(getIsBranch());
			assert(childOffset < getBranchChildCount());
			return getBranchChildren()[childOffset];
		}
		
	public:

		bool getIsLeaf(Value & value) const
		{
			if (getIsLeaf())
			{
				value = getLeafValue();
				return true;
			}
			return false;
		}

		// Returns true if this is a leaf with no edge.
		bool getIsFull() const
		{
			return getIsLeaf() && !getEdgeSize();
		}

		// Returns true if this is a leaf with no edge.
		bool getIsFull(Value & value) const
		{
			return getIsLeaf(value) && !getEdgeSize();
		}

		// Sets the node to empty.
		// Deletes old vertex.
		void setIsEmpty()
		{
			if (!getIsEmpty())
			{
				if (getIsLeaf())
				{
					destructLeafVertex();
				} else
				{
					destructBranchVertex();
				}

				free(array);
				array = 0;
			}
		}
		
		// Sets the node to full.
		void setIsFull(Value value = Value())
		{
			Node leaf(0, (char unsigned const *)0, value);
			swap(leaf);
		}

	private:

		// Constructs a leaf with default value.
		explicit Node(
			char const edgeSize,
			char unsigned const * const edge,
			Value value = Value()) :
		array()
		{
			array = (char unsigned *)malloc(getEdgeOffset() + edgeSize + valueSize);
			if (!array)
			{
				throw std::bad_alloc();
			}

			// Initialize the header: leaf bit + edge size.
			*array = (edgeSize | headerIsLeafMask);
			
			// Copy the edge.
			if (edgeSize)
			{
				assert(edge);
				memcpy(array + getEdgeOffset(), edge, edgeSize);
			}

			// Construct the leaf vertex.
			constructLeafVertex(value);
		}
			
		// Constructs a branch whose children are all empty.
		// The child count argument is first, rather than last,
		// to avoid overload conflict with leaf constructor
		// when Value = Step.
		explicit Node(
			char childCount,
			char const edgeSize,
			char unsigned const * const edge) :
		array()
		{
			assert(1 < childCount);
		
			array = (char unsigned *)malloc(
				getEdgeOffset() +
				edgeSize +
				computeBranchVertexSize(childCount));
			if (!array)
			{
				throw std::bad_alloc();
			}

			// Initialize the header: branch bit + edge size.
			*array = edgeSize;

			// Copy the edge.
			if (edgeSize)
			{
				assert(edge);
				memcpy(array + getEdgeOffset(), edge, edgeSize);
			}

			// Construct the branch vertex with empty child nodes.
			constructBranchVertex(childCount);
		}

	public:

		// Constructs an empty node.
		explicit Node() : array() {}

		// Copies the node.
		Node(Node const & node) : array()
		{
			if (!node.getIsEmpty())
			{
				bool const isLeaf = node.getIsLeaf();
				size_t const vertexOffset = node.getVertexOffset();
				
				char childCount = isLeaf ? 0 : node.getBranchChildCount();
				size_t const vertexSize = node.getVertexSize();

				// Allocate this array, and copy the prefix (header + edge) into it.
				array = (char unsigned *)malloc(vertexOffset + vertexSize);
				memcpy(array, node.array, vertexOffset);

				// Placement copy the vertex contents.
				if (isLeaf)
				{
					constructLeafVertex(*(Value const *)(node.array + vertexOffset));
				} else
				{
					constructBranchVertex(childCount, node.getBranchChildren());
				}
			}
		}

		~Node()
		{
			setIsEmpty();
		}

		void swap(Node & with)
		{
			// Swap the array pointer.
			boost::swap(array, with.array);
		}

		Node & operator =(Node node)
		{
			swap(node);
			return *this;
		}

		// Inserts a subtree, mapped to value.
		// Returns false if the caller needs to check for consolidation.
		bool insert(Index const & index, char indexStepOffset, char const indexStepCount,
			Value value = Value())
		{
			assert(indexStepOffset <= indexStepCount);

			// Assert that the index has more than one child.
			// (This should have been handled by the calling SubtreeMap method.)
			assert(0 == indexStepCount || 1 < index.getChildCount(Level(indexStepCount - 1)));

			// If empty, construct.
			if (getIsEmpty())
			{
				// Construct a leaf from the remainder of the index,
				// and swap this node with the leaf.
				char const edgeSize = indexStepCount - indexStepOffset;
				Node leaf(
					edgeSize,
					(indexStepOffset < indexStepCount) ? &(index.getSteps()[indexStepOffset]) : 0,
					value);
				swap(leaf);

				return 0 < edgeSize;
			}
			assert(!getIsEmpty());
			
			// Iterate through steps in edge.
			char const edgeSize = getEdgeSize();
			char unsigned const * const edge = array + getEdgeOffset();
			char edgeOffset = 0;
			for (; edgeOffset < edgeSize; ++edgeOffset, ++indexStepOffset)
			{
				if (indexStepOffset == indexStepCount)
				{
					// Set to a leaf, with edge size truncated to edgeOffset.
					// The index has already been trimmed.
					Node leaf(edgeOffset, edge, value);
					swap(leaf);

					assert(getEdgeSize() == edgeOffset);
					return 0 < edgeOffset;
				}
				
				// If the step is different, branch and return false if consolidatable.
				assert(indexStepCount);
				assert(indexStepOffset < indexStepCount);
				Step const indexStep = index.getSteps()[indexStepOffset];
				if (edge[edgeOffset] != indexStep)
				{
					// Get the parent index child count.
					char const childCount = index.getChildCount(Level(indexStepOffset));

					// We can pre-consolidate if:
					//	-	This is a leaf
					//	-	The existing and new values are the same
					//	-	The index child count is 2
					//	-	The edge and the index only have one step left
					if (2 == childCount &&
						indexStepOffset + 1 == indexStepCount &&
						edgeOffset + 1 == edgeSize)
					{
						Value existingValue;
						if (getIsLeaf(existingValue) && existingValue == value)
						{
							// Walk back edge size as long as parent child count is 1.
							while (edgeOffset && 1 == index.getChildCount(Level(--indexStepOffset)))
							{
								--edgeOffset;
							}

							// Shrink the leaf; new edge size is 'edgeOffset'.
							destructLeafVertex();
							if (!(array = (unsigned char *)realloc(array, getEdgeOffset() + edgeOffset + valueSize)))
							{
								throw std::bad_alloc();
							}
							*array = (edgeOffset | headerIsLeafMask); // Set header (size + leaf bit).
							constructLeafVertex(existingValue);

							// If the new edge size is 0, there may be a possibility
							// of further consolidation.
							assert(getEdgeSize() == edgeOffset);
							return (0 < edgeOffset);
						}
					}
				
					// Split the edge at edge offset, and insert the rest as a child.
					{
						assert(!getIsEmpty());
						assert(edgeOffset < getEdgeSize());

						// Get child offset.
						Step const childOffset = edge[edgeOffset];

						// Create child node, copying end part from this node's edge, 
						// destruct vertex, and convert to branch.
						Node child;
						char childEdgeOffset = edgeOffset + 1; // Skip child offset.
						char childEdgeSize = edgeSize - childEdgeOffset;
						Value value;
						if (getIsLeaf(value))
						{
							// Create a child leaf from
							// the left-over part of the edge and the existing value.
							Node(childEdgeSize, edge + childEdgeOffset, value).swap(child);
							
							destructLeafVertex();
						} else
						{
							assert(getIsBranch());

							// Create a child branch from the left-over part of the edge, and with the same 
							// child count (and empty children).
							Node(getBranchChildCount(), childEdgeSize, edge + childEdgeOffset).swap(child);

							// Swap my children with child branch's empty children.
							swapBranchChildren(child);
							
							destructBranchVertex();
						}
						*array = edgeOffset; // Set header (size, no leaf bit).
						if (!(array = (unsigned char *)realloc(array, getEdgeOffset() + edgeOffset + computeBranchVertexSize(childCount))))
						{
							throw std::bad_alloc();
						}
						constructBranchVertex(childCount); // Results in empty children.

						// Swap the new child node into the proper child slot.
						getBranchChild(childOffset).swap(child);
					}

					// Insert the rest of the index as a child leaf.
					++indexStepOffset; // Skip child step.
					Node leaf(
						indexStepCount - indexStepOffset, 
						(indexStepOffset < indexStepCount) ? &(index.getSteps()[indexStepOffset]) : 0,
						value);
					getBranchChild(indexStep).swap(leaf);
					
					return true;
				}
			}

			// If at end of index: make this a leaf, and set the value.
			assert(edgeSize == edgeOffset);
			assert(indexStepOffset <= indexStepCount);
			if (indexStepCount == indexStepOffset)
			{
				// Although every index received is trimmed, this can still be a branch if
				// the index is empty.
				if (getIsLeaf())
				{
					setLeafValue(value);
					return 0 < edgeSize;
				}
			} else
			{
				// The index continues on.
				// Get the next step.
				assert(indexStepOffset < indexStepCount);
				Step const * indexSteps = &index.getSteps()[indexStepOffset];

				Value existingValue;
				if (getIsLeaf(existingValue))
				{
					// If the value is not the same, insert it.
					// Otherwise it is implicitly contained, so it is a no-op.
					if (value != existingValue)
					{
						// Get the child count.
						char childCount = index.getChildCount(Level(indexStepOffset));
						assert(childCount);

						// Set this to a branch, with all children empty.
						destructLeafVertex();
						if (1 == childCount)
						{
							// Assert that the index carries on; otherwise, it would have been pruned.
							assert(indexStepOffset < (indexStepCount - 1));

							// Advance index offset to next step and get new child count.
							childCount = index.getChildCount(Level(++indexStepOffset));
							assert(1 < childCount);

							// We need to extend the edge to encompass the single child.
							// We know that the index carries on past this, and that the next child is not
							// a single child.
							if (!(array = (unsigned char *)realloc(array,
										getVertexOffset() + 1 + computeBranchVertexSize(childCount))))
							{
								throw std::bad_alloc();
							}
							*array = edgeSize + 1; // Clear leaf bit and set the edge size to the new one. 
							array[getEdgeOffset() + edgeSize] = *indexSteps;

							// Advance index step array pointer.
							++indexSteps;
						} else
						{
							if (!(array = (unsigned char *)realloc(array,
										getVertexOffset() + computeBranchVertexSize(childCount))))
							{
								throw std::bad_alloc();
							}
							*array &= ~headerIsLeafMask; // Clear leaf bit in header.
						}
						constructBranchVertex(childCount);

						// Make all siblings full leaves, with value 'existingValue'.
						while (*indexSteps < --childCount)
						{
							getBranchChild(childCount).setIsFull(existingValue);
						}
						while (0 < childCount)
						{
							getBranchChild(--childCount).setIsFull(existingValue);
						}

						// Insert the rest of the index as a child leaf.
						++indexStepOffset; // Skip child step.
						assert(indexStepOffset <= indexStepCount);
						Node leaf(
							indexStepCount - indexStepOffset, 
							indexSteps,
							value);
						getBranchChild(*indexSteps).swap(leaf);
					}
					return true;
				}

				// Get the child and call insert on it.
				// If it returns false, consolidate as possible and return bool accordingly.
				assert(getIsBranch());
				Node & child = getBranchChild(*indexSteps);
				if (child.insert(index, indexStepOffset + 1, indexStepCount, value))
				{
					return true;
				}

				assert(!child.getIsEmpty() && !child.getEdgeSize());

				// If all children are full, and values are the same, can consolidate.
				Value childValue;
				for (char childCount = getBranchChildCount(); 0 < childCount; )
				{
					// Return true if we cannot consolidate.
					if (!getBranchChild(--childCount).getIsFull(childValue) || childValue != value)
					{
						return true;
					}
				}
			}
				
			// Walk back edge size as long as parent child count is 1.
			assert(getIsBranch());
			assert(edgeSize == edgeOffset);
			while (edgeOffset && 1 == index.getChildCount(Level(--indexStepOffset)))
			{
				--edgeOffset;
			}

			// Convert branch to leaf.  New edge size is 'edgeOffset'.
			destructBranchVertex();
			if (edgeOffset < edgeSize &&
				!(array = (unsigned char *)realloc(array, getEdgeOffset() + edgeOffset + valueSize)))
			{
				throw std::bad_alloc();
			}
			*array = (edgeOffset | headerIsLeafMask); // Set leaf bit in header.
			constructLeafVertex(value);

			assert(getEdgeSize() == edgeOffset);
			return 0 < edgeOffset;
		}
		
		// Removes a subtree.
		// Returns false if caller should check for streamlining.
		bool remove(Index const & index, char indexStepOffset, char const indexStepCount)
		{
			assert(indexStepOffset <= indexStepCount);

			// Assert that the index has more than one child.
			// (This should have been handled by the calling SubtreeMap method.)
			assert(0 == indexStepCount || 1 < index.getChildCount(Level(indexStepCount - 1)));

			if (getIsEmpty())
			{
				return true;
			}

			// If the index ends within the edge, whether it matches or not,
			// it is not contained and doesn't need to be removed.
			assert(!getIsEmpty());
			char const edgeSize = getEdgeSize();
			if (indexStepCount < (indexStepOffset + edgeSize))
			{
				return true;
			}

			// Iterate through steps in edge and increment indexCurrent.
			// If any step is different, the index is not contained.
			// At completion, indexCurrent will have been incremented by edgeSize.
			char unsigned const * const edge = array + getEdgeOffset();
			for (char edgeOffset = 0; edgeOffset < edgeSize; ++edgeOffset,
				++indexStepOffset)
			{
				// If next in index is different, it is not contained.
				assert(indexStepOffset < indexStepCount);
				if (edge[edgeOffset] != index.getSteps()[indexStepOffset])
				{
					return true;
				}
			}
				
			// If indexStepOffset is now at the end of the index
			// (i.e. index doesn't extend past edge):
			assert(indexStepOffset <= indexStepCount);
			if (indexStepCount == indexStepOffset)
			{
				// If this is a leaf, then it is contained; clear this leaf.
				if (getIsLeaf())
				{
					setIsEmpty();
					return false;
				}

				// This is a branch; it is not contained.
				assert(getIsBranch());
				return true;
			}

			// The index continues on.
			// Get the next step.
			assert(indexStepOffset < indexStepCount);
			Step indexStep = index.getSteps()[indexStepOffset];
			
			// If this is a leaf, 
			// insert all siblings and give them all the same value as this.
			// Get a copy of the value before its memory goes away.
			Value value;
			if (getIsLeaf(value))
			{
				// Copy this node's edge into the edge vector.
				std::vector< Step > edgeSteps(edge, edge + edgeSize);

				// Iterate through the remaining steps in the index and create full nodes
				// for all siblings at each level.
				assert(indexStepOffset < indexStepCount);
				Node * node = this;
				do
				{
					// Get the ancestor step.
					Step ancestor = index.getSteps()[indexStepOffset];
					
					// Get the parent child count.
					char parentChildCount = index.getChildCount(Level(indexStepOffset));
					assert(parentChildCount);
					switch (parentChildCount)
					{
					case 1:
						// Append the ancestor to the local edge vector.
						edgeSteps.push_back(ancestor);
						break;
					default:
						// Construct a branch, with the edge from the edge vector,
						// and swap it with the current node.
						Node(parentChildCount, edgeSteps.size(),
							edgeSteps.empty() ?
							(char unsigned const *)0 :
							&edgeSteps.front()).swap(*node);

						// Clear local edge vector.
						edgeSteps.clear();
						
						// For each child that is not ancestor, set is full.
						while (ancestor < --parentChildCount)
						{
							node->getBranchChild(parentChildCount).setIsFull(value);
						}
						while (0 < parentChildCount)
						{
							node->getBranchChild(--parentChildCount).setIsFull(value);
						}

						// Set the current node to the child ancestor.
						node = &node->getBranchChild(ancestor);
					}
				} while (++indexStepOffset < indexStepCount);
				return true;
			}

			// This is a branch, and the index continues; defer to child.
			assert(getIsBranch());
			{
				Node & child = getBranchChild(indexStep);
				if (child.remove(index, indexStepOffset + 1, indexStepCount))
				{
					return true;
				}
				assert(child.getIsEmpty());
			}

			// If there is more than one non-empty child, return true.
			boost::optional< Step > singleNonEmptyChildOffset;
			for (char childCount = getBranchChildCount(); 0 < childCount; )
			{
				if (!getBranchChild(--childCount).getIsEmpty())
				{
					if (singleNonEmptyChildOffset)
					{
						// There is more than one non-empty child.
						return true;
					}
					singleNonEmptyChildOffset = childCount;
				}
			}

			// At this point, there should only be one non-empty child;
			// there should never be none, because this would have been
			// merged into the edge.
			// Merge the non-empty child into this node.
			// Swap out the desired child with empty.
			// Now there is a local orphaned copy that will 
			// be destroyed at the end of the scope.
			assert(singleNonEmptyChildOffset);
			assert(*singleNonEmptyChildOffset < getBranchChildCount());
			{
				Node child;
				child.swap(getBranchChild(*singleNonEmptyChildOffset));
				assert(!child.getIsEmpty());
				
				// Compute new edge size: current + 1 (for child offset) + child edge size.
				char const newEdgeSize = edgeSize + 1 + child.getEdgeSize();

				// Destruct the vertex (child array),
				// and realloc to make room in edge
				// (for child offset and child edge)
				// and for the child's vertex size.
				destructBranchVertex();
				if (!(array = (unsigned char *)realloc(array, getEdgeOffset() + newEdgeSize + child.getVertexSize())))
				{
					throw std::bad_alloc();
				}
				array[getEdgeOffset() + edgeSize] = *singleNonEmptyChildOffset; // Copy child offset to edge.
				memcpy(array + getEdgeOffset() + edgeSize + 1, child.getEdge(), child.getEdgeSize()); // 1 for the child offset.
				if (child.getIsLeaf())
				{
					// Set leaf bit and new edge size in header.
					*array = (newEdgeSize | headerIsLeafMask);

					constructLeafVertex(child.getLeafValue());
				} else
				{
					assert(child.getIsBranch());

					// Set new edge size, without leaf bit, in header.
					*array = newEdgeSize;

					// Construct branch vertex with empty children,
					// and count matching child's child count.
					constructBranchVertex(child.getBranchChildCount());
					assert(getBranchChildCount() == child.getBranchChildCount());

					// Swap the child pointers.
					swapBranchChildren(child);
				}
			}
			return true;
		}
		
		// Returns true if the subtree is found (and sets the value argument), and false if not.
		// If found, updates indexStepCount to the level of the index found, which will be an inclusive
		// ancestor of the original index.
		bool find(Index const & index, char indexStepOffset, char & indexStepCount,
			Value & value) const
		{
			indexStepCount = index.getStepCount().offset;

			if (getIsEmpty())
			{
				return false;
			}

			// Check edge.
			{
				// If the index ends within the edge, whether it matches or not,
				// it is not contained.
				char const edgeSize = getEdgeSize();
				if (indexStepCount < (indexStepOffset + edgeSize))
				{
					return false;
				}

				// For each step in edge:
				char unsigned const * edge = array + getEdgeOffset();
				for (char edgeOffset = 0; edgeOffset < edgeSize;
					++edgeOffset, ++indexStepOffset)
				{
					// If the next step in the index is different,
					// it is not contained.
					assert(indexStepOffset < indexStepCount);
					if (edge[edgeOffset] != index.getSteps()[indexStepOffset])
					{
						return false;
					}
				}
			}

			// If it is a leaf, set the value and index step count, and return true.
			if (getIsLeaf(value))
			{
				indexStepCount = indexStepOffset;
				return true;
			}

			// If the index continues, defer to child.
			assert(getIsBranch());
			if (indexStepOffset < indexStepCount)
			{
				Step const step = index.getSteps()[indexStepOffset]; // Get step before incrementing offset.
				return getBranchChild(step).find(
					index, ++indexStepOffset, indexStepCount, value);
			}

			return false;
		}
	} node;

	// Helper function.
	// Drop single-children off end of index by adjusting level down.
	static Level trimSingleChildren(Index const & index)
	{
		// Drop single-children off end of index by adjusting level down.
		Level stepCount = index.getStepCount();
		while (stepCount.offset && 1 == index.getChildCount(Level(stepCount.offset - 1)))
		{
			--stepCount.offset;
		}
		return stepCount;
	}

public:

	class Test;

	typedef boost::compressed_pair< Index const &, Value > Pair;

	// A collection of subtree/value pairs in a subtree map.
	// This may be filtered to exclude branches.
	class Pairs : public ForwardRangeInterface< Pair const & >
	{
	public:

		typedef FunctorInterface< bool, Index const & > Filter;

		// Convenience filter for iteration to a maximum step count.
		class StepCountFilter : public Filter
		{
			Level stepCount;

		public:

			explicit StepCountFilter(Level stepCount) : stepCount(stepCount) {}

			bool operator ()(Index const & index)
			{
				return (stepCount.offset < index.getStepCount().offset);
			}
		};

	private:

		struct PositionNode
		{
			typename Node::ChildIterator childIterator;
			Level stepCount;
			
			explicit PositionNode(typename Node::ChildIterator childIterator) :
			childIterator(childIterator),
			stepCount(0)
			{}
		};

		typedef std::vector< PositionNode > Position;

		// The current position.
		// If empty, the range is empty.
		Position position;

		// Returns true to filter (exclude) the branch.
		// This is called in two cases:
		//	-	The index gets a step appended.
		//	-	The index gets 0 or more steps removed from the end,
		//		and the new last step is changed.
		Filter * filter;

		// The current index.
		Index index;

		// The current pair.
		Pair pair;

		// Returns true if filtered; false if not.
		bool getIsFiltered() const
		{
			return filter && (*filter)(index);
		}

		void finish()
		{
			position.clear();
			index.setIsEmpty();
			filter = 0;
		}

		// Roll back the index to exclude the back position node,
		// and pop it.
		void ascend()
		{
			assert(!position.empty());

			PositionNode & positionNode = position.back();

			// Roll back index to drop the steps for the back position node.
			index.stepToAncestor(positionNode.stepCount);

			// Roll back index to drop the child offset for the back position node.
			index.stepToParent();

			position.pop_back();
		}

		// Starting at the index given by the back position node,
		// descend to the first leaf.
		// Assumes the index is populated with steps down to, and including,
		// the offset of the child iterator in the back position node.
		// Returns false if caller needs to advance (due to branch filtering).
		bool descend()
		{
			assert(!position.empty());

			// Get the back position node.
			// Assert that the position node's child iterator is not at the end,
			// and that the position node's step count is 0.
			PositionNode & positionNode = position.back();
			assert(positionNode.childIterator != typename Node::ChildIterator());
			assert(0 == positionNode.stepCount.offset);

			// Get the node that the back position node's child iterator is pointing to.
			Node const & node = *positionNode.childIterator;
			assert(!node.getIsEmpty());

			// Populate the index with the steps,
			// and update step count on the back position node.
			for (typename Node::StepIterator stepIterator(node);
				stepIterator != typename Node::StepIterator(); ++stepIterator)
			{
				index.stepToChild(*stepIterator);
				++positionNode.stepCount.offset;
				if (getIsFiltered())
				{
					return false;
				}
			}
			
			// If it's a leaf, populate the pair value and return.
			if (node.getIsLeaf(pair.second()))
			{
				return true;
			}

			// Descend each child until we get a non-filtered one,
			// or end.
			assert(node.getIsBranch());
			for (typename Node::ChildIterator childIterator(node);
				childIterator != typename Node::ChildIterator(); ++childIterator)
			{
				index.stepToChild(childIterator.getOffset());
				position.push_back(PositionNode(childIterator));
				if (!getIsFiltered() && descend())
				{
					// We had a successful descent to a leaf.  Return.
					return true;
				}
				ascend();
			}

			return false;
		}

		void advance()
		{
			for (; ; )
			{
				assert(!position.empty());

				// Get a copy of the child iterator.
				typename Node::ChildIterator childIterator = position.back().childIterator;
				assert(childIterator != typename Node::ChildIterator());
				
				// Roll back and pop the back position node.
				ascend();

				// Advance child iterator.
				if (++childIterator == typename Node::ChildIterator())
				{
					if (position.empty())
					{
						finish();
						return;
					}
				} else
				{
					index.stepToChild(childIterator.getOffset());
					position.push_back(PositionNode(childIterator));
					if (!getIsFiltered() && descend())
					{
						// Not filtered, and we can descend all the way down; return.
						return;
					}
				}
			}
		}

	public:

		explicit Pairs(
			SubtreeMap const & subtreeMap,
			Filter * const filter = 0) :
		position(),
		filter(filter),
		index(),
		pair(index, boost::initialized_value)
		{
			// If the root node is filtered or empty, the range is empty.
			if (getIsFiltered() || subtreeMap.getIsEmpty())
			{
				finish();
				return;
			}
			
			position.push_back(
				PositionNode(
					typename Node::ChildIterator(&subtreeMap.node, &subtreeMap.node + 1)));
			if (!descend())
			{
				finish();
			}
		}

		bool getIsEmpty() const
		{
			return position.empty();
		}

		Pair const & getFront() const
		{
			return pair;
		}

		// Moves to next node, in depth-first order, for which filter returns false.
		// If the filter returns true for a node, its entire tree is skipped.
		// Stepping past the end is undefined.
		void popFront()
		{
			assert(!getIsEmpty());
			advance();
		}
		
		size_t getCount() const
		{
			// Copy the current pairs, then count remainder.
			size_t count = 0;
			for (Pairs pairs(*this); !pairs.getIsEmpty(); pairs.popFront())
			{
				++count;
			}
			return count;
		}
	};

	// Uses the return value optimization.
	Pairs getPairs(typename Pairs::Filter * const filter = 0) const
	{
		return Pairs(*this, filter);
	}

	void swap(SubtreeMap & with)
	{
		node.swap(with.node);
	}

	// Returns true if the tree is empty.
	bool getIsEmpty() const
	{
		return node.getIsEmpty();
	}

	// Makes the tree empty.
	void setIsEmpty()
	{
		node.setIsEmpty();
	}

	// Returns true if the tree only contains the root index.
	bool getIsFull() const
	{
		return node.getIsFull();
	}

	void setIsFull(Value value)
	{
		insert(Index(), value);
	}

	void setIsFull()
	{
		setIsFull(Value());
	}

	// Inserts the subtree, implicitly or explicitly, and sets its value.
	// 	-	If index is a branch, split and set.
	//		Parent may consolidate (if only 2 children, and both are leaves with same value).
	// 	-	If index is a leaf, sets the value.
	//		Parent may consolidate (all children are leaves with same value).
	// 	-	If index is a leaf descendant:
	//		-	If the value differs from leaf:
	//			-	Insert leaf with value, and insert siblings with leaf value.
	//		-	Else:
	//			-	No change; already implicitly included.
	void insert(Index const & index, Value value = Value())
	{
		Level stepCount = trimSingleChildren(index);
		node.insert(index, 0, stepCount.offset, value);
	}

	// Removes the subtree, explicitly or implicitly.
	// 	-	If cell is a branch, no change.
	// 	-	If cell is a leaf, removes.
	//		Parent may streamline (if results in single child).
	// 	-	If cell is a leaf descendant, adds siblings with value of leaf.
	//		Parent may streamline (if only one sibling).
	void remove(Index const & index)
	{
		Level stepCount = trimSingleChildren(index);
		node.remove(index, 0, stepCount.offset);
	}

	// If the subtree defined by the index is found, implicitly or explicitly,
	// the optional value and stepCount arguments are set accordingly.
	// Otherwise, returns false.
	bool find(Index const & index, Value * value = 0, Level * stepCount = 0) const
	{
		Value valueOut;
		char stepCountOut;
		if (node.find(index, 0, stepCountOut, valueOut))
		{
			if (value) { *value = valueOut; }
			if (stepCount) { stepCount->offset = stepCountOut; }
			return true;
		}
		return false;
	}

	// Returns the descendant cell count at the given step count.
	size_t getDescendantCount(Level stepCount) const
	{
		size_t result = 0;

		typename Pairs::StepCountFilter stepCountFilter(stepCount);
		for (Pairs pairs(*this, &stepCountFilter);
			!pairs.getIsEmpty(); pairs.popFront())
		{
			Index const & index = pairs.getFront().first();
			Level indexStepCount = index.getStepCount();
			assert(indexStepCount.offset <= stepCount.offset);

			result += index.getDescendantCount(
				Tree::Level(stepCount.offset - indexStepCount.offset));
		}

		return result;
	}

	// Constructs an empty subtree map.
	explicit SubtreeMap() : node() {}

	// Reads the subtree map from a stream.
	explicit SubtreeMap(std::istream & istream);

	~SubtreeMap()
	{
		setIsEmpty();
	}
};

template < typename Value >
class Pyxis::Globe::Tree::SubtreeMap< Value >::Test
{
	Index const index;
	Index const index0;
	Index const index00;
	Index const index000;
	Index const index001;
	Index const index002;
	Index const index003;
	Index const index004;
	Index const index005;
	Index const index0010;
	Index const index1;
	Index const index10;
	Index const index100;
	Index const index101;
	Index const index102;
	Index const index103;
	Index const index104;
	Index const index105;
	Index const index1020;
	Index const index1030;
	Index const index1040;
	Index const index10400;
	Index const index10401;
	Index const index104000;
	Index const index104001;
	Index const index104002;
	Index const index104003;
	Index const index104004;
	Index const index104005;

	bool testDefaultConstruction() const
	{
		SubtreeMap subtreeMap;
		if (!subtreeMap.getIsEmpty())
		{
			return PYXIS__ASSERT___FAIL(
				"The default constructed map was not empty.");
		}
		return true;
	}
	
	bool testInsertValue() const
	{
		int value;

		// insert(Value).
		SubtreeMap< int > subtreeMap;
		if (subtreeMap.find(Index()))
		{
			return PYXIS__ASSERT___FAIL(
				"The value of a newly-constructed map must be default.");
		}
		subtreeMap.insert(Index(), 42);
		if (!subtreeMap.find(Index(), &value) || value != 42)
		{
			return PYXIS__ASSERT___FAIL(
				"The set value was not correctly retrieved.");
		}

		subtreeMap.remove(Index());
		if (subtreeMap.find(Index()))
		{
			return PYXIS__ASSERT___FAIL(
				"The removed value should not be in the set.");
		}
		if (!subtreeMap.getIsEmpty())
		{
			return PYXIS__ASSERT___FAIL(
				"The map should be empty.");
		}
		
		// insert(Index, Value).
		subtreeMap.insert(index10401, 19); // Result: 10401:19
		if (!subtreeMap.find(index10401, &value) || value != 19)
		{
			return PYXIS__ASSERT___FAIL(
				"The set value was not correctly retrieved.");
		}
		if (subtreeMap.getPairs().getCount() != 1)
		{
			return PYXIS__ASSERT___FAIL(
				"The leaf count is incorrect.");
		}

		// insert(Index, Value): basic testing.
		subtreeMap.insert(index0010, 401); // Result: 001:401, 10401:19
		if (!subtreeMap.find(index0010, &value) || value != 401)
		{
			return PYXIS__ASSERT___FAIL(
				"The set value was not correctly retrieved.");
		}
		if (!subtreeMap.find(index10401, &value) || value != 19)
		{
			return PYXIS__ASSERT___FAIL(
				"The set value was not correctly retrieved.");
		}
		if (subtreeMap.getPairs().getCount() != 2)
		{
			return PYXIS__ASSERT___FAIL(
				"The leaf count is incorrect.");
		}
		
		// Set one to a new value.
		subtreeMap.insert(index10401, 0); // Result: 001:401, 10401:0
		if (!subtreeMap.find(index10401, &value) || value != 0)
		{
			return PYXIS__ASSERT___FAIL(
				"The set value was not correctly retrieved.");
		}
		if (subtreeMap.getPairs().getCount() != 2)
		{
			return PYXIS__ASSERT___FAIL(
				"The leaf count is incorrect.");
		}
		
		// Set to a new value again.
		subtreeMap.insert(index10401, 404); // Result: 001:401, 10401:404
		if (!subtreeMap.find(index10401, &value) || value != 404)
		{
			return PYXIS__ASSERT___FAIL(
				"The set value was not correctly retrieved.");
		}
		if (subtreeMap.getPairs().getCount() != 2)
		{
			return PYXIS__ASSERT___FAIL(
				"The leaf count is incorrect.");
		}
		
		// Set to default value.
		subtreeMap.insert(index10401); // Result: 001:401, 10401:0
		if (!subtreeMap.find(index10401, &value) || value != 0)
		{
			return PYXIS__ASSERT___FAIL(
				"The set value was not correctly retrieved.");
		}
		if (subtreeMap.getPairs().getCount() != 2)
		{
			return PYXIS__ASSERT___FAIL(
				"The leaf count is incorrect.");
		}

		// insert(Index, Value): argument as subset of trunk should branch.
		subtreeMap.insert(index000, 402); // Result: 000:402, 001:401, 10401:0
		if (!subtreeMap.find(index0010, &value) || value != 401)
		{
			return PYXIS__ASSERT___FAIL(
				"The original value was not correctly retrieved.");
		}
		if (!subtreeMap.find(index000, &value) || value != 402)
		{
			return PYXIS__ASSERT___FAIL(
				"The set value was not correctly retrieved.");
		}
		if (subtreeMap.getPairs().getCount() != 3)
		{
			return PYXIS__ASSERT___FAIL(
				"The value count is incorrect.");
		}

		// Other consolidation-related tests.
		subtreeMap.insert(index00); // Result: 00:0, 10401:0
		if (!subtreeMap.find(index00))
		{
			return PYXIS__ASSERT___FAIL(
				"The leaf was not found.");
		}
		if (!subtreeMap.find(index00, &value) || value != 0)
		{
			return PYXIS__ASSERT___FAIL(
				"The value was not correctly retrieved.");
		}
		if (subtreeMap.find(index1))
		{
			return PYXIS__ASSERT___FAIL(
				"The index should not be present.");
		}
		subtreeMap.insert(index1, 1); // Result: 1:1, 00:0, 10401:0
		if (!subtreeMap.find(index00))
		{
			return PYXIS__ASSERT___FAIL(
				"The leaf was not found.");
		}
		if (!subtreeMap.find(index00, &value) || value != 0)
		{
			return PYXIS__ASSERT___FAIL(
				"The value was not correctly retrieved.");
		}
		if (!subtreeMap.find(index1, &value) || value != 1)
		{
			return PYXIS__ASSERT___FAIL(
				"The value was not correctly retrieved.");
		}
		
		// insert(Index, Value): argument matching [empty] trunk should not consolidate.
		// Create a consolidatable scenario and verify that it doesn't happen.
		// Index {00x}, where x is everything but 0 and all leaves.  Then set value {000} to a non-zero.
		subtreeMap.setIsEmpty();
		if (!subtreeMap.getIsEmpty())
		{
			return PYXIS__ASSERT___FAIL(
				"The tree should be empty.");
		}
		{
			// Insert leaves for 00x, where x is everything but 0.
			Index shortIndex("00");
			char childCount(shortIndex.getChildCount());

			for (Step offset(0); ++offset < childCount; )
			{
				Index index(shortIndex);
				index.stepToChild(offset);
				subtreeMap.insert(index);
				if (!subtreeMap.find(index))
				{
					return PYXIS__ASSERT___FAIL(
						"The leaf was not found.");
				}
			}
			assert(childCount);
			if (subtreeMap.getPairs().getCount() != (Step)(childCount - 1))
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf count is incorrect.");
			}
			
			// Set non-default value for 000, and ensure that consolidation doesn't happen.
			// Set non-default value for 000 again, and ensure that consolidation still doesn't happen.
			Index index(shortIndex);
			index.stepToChild(0);
			for (int repeat = 0; repeat < 2; ++repeat)
			{
				subtreeMap.insert(index, 42);
				if (!subtreeMap.find(index, &value) || value != 42)
				{
					return PYXIS__ASSERT___FAIL(
						"The value was not correctly set.");
				}
				if (subtreeMap.getPairs().getCount() != (size_t const)childCount)
				{
					return PYXIS__ASSERT___FAIL(
						"The leaf count is incorrect.");
				}
			}

			// Set default value for 000, and ensure that consolidation to 0 does happen.
			subtreeMap.insert(index, 0);
			if (subtreeMap.getPairs().getCount() != 1)
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf count is incorrect.");
			}
			if (!subtreeMap.find(index0))
			{
				return PYXIS__ASSERT___FAIL(
					"Consolidation was incorrect.");
			}

			// Insert a consolidatable index, with a non-consolidatable value.
			subtreeMap.insert(index1, 1);
			if (subtreeMap.getPairs().getCount() != 2)
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf count is incorrect.");
			}
			if (!subtreeMap.find(index0, &value) || value != 0)
			{
				return PYXIS__ASSERT___FAIL(
					"Consolidation was incorrect.");
			}
			if (!subtreeMap.find(index1, &value) || value != 1)
			{
				return PYXIS__ASSERT___FAIL(
					"Consolidation was incorrect.");
			}
			
			// Set the value to 0 and consolidate.
			subtreeMap.insert(index1, 0);
			if (!subtreeMap.getIsFull())
			{
				return PYXIS__ASSERT___FAIL(
					"The tree should have consolidated and streamlined to full.");
			}

			// Set non-default value for 000; make sure it inserts a node.
			subtreeMap.insert(index, 69);
			if (!subtreeMap.find(index, &value) || value != 69)
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf has the wrong value.");
			}
			if (subtreeMap.getPairs().getCount() != 2)
			{
				return PYXIS__ASSERT___FAIL(
					"Wrong count.");
			}
			
			subtreeMap.insert(index0, 69);
			if (!subtreeMap.find(index0, &value) || value != 69)
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf has the wrong value.");
			}
			if (subtreeMap.getPairs().getCount() != 2)
			{
				return PYXIS__ASSERT___FAIL(
					"Wrong count.");
			}
			
			subtreeMap.insert(index1, 69);
			if (!subtreeMap.getIsFull())
			{
				return PYXIS__ASSERT___FAIL(
					"Didn't consolidate.");
			}
			if (!subtreeMap.find(Index(), &value) || value != 69)
			{
				return PYXIS__ASSERT___FAIL(
					"Wrong value.");
			}
		}

		subtreeMap.setIsEmpty();
		subtreeMap.insert(index00, 1); // Result: 0:1
		if (subtreeMap.getPairs().getCount() != 1)
		{
			return PYXIS__ASSERT___FAIL(
				"Wrong count.");
		}
		subtreeMap.insert(index104, 2); // Result: 0:1, 104:2
		if (subtreeMap.getPairs().getCount() != 2)
		{
			return PYXIS__ASSERT___FAIL(
				"Wrong count.");
		}
		subtreeMap.insert(index004, 3); // Result: 000:1, 001:1, 002:1, 003:1, 004:3, 005:1, 104:2
		if (subtreeMap.getPairs().getCount() != 7)
		{
			return PYXIS__ASSERT___FAIL(
				"Wrong count.");
		}
		if (!subtreeMap.find(index001, &value) || value != 1)
		{
			return PYXIS__ASSERT___FAIL(
				"Wrong contents.");
		}
		if (!subtreeMap.find(index004, &value) || value != 3)
		{
			return PYXIS__ASSERT___FAIL(
				"Wrong contents.");
		}
		subtreeMap.insert(index00, 4); // Result: 0:4, 104:2
		if (subtreeMap.getPairs().getCount() != 2)
		{
			return PYXIS__ASSERT___FAIL(
				"Wrong count.");
		}
		if (!subtreeMap.find(index0, &value) || value != 4)
		{
			return PYXIS__ASSERT___FAIL(
				"Wrong contents.");
		}
		subtreeMap.insert(Index(), 5); // Result: {}:5
		if (subtreeMap.getPairs().getCount() != 1)
		{
			return PYXIS__ASSERT___FAIL(
				"Wrong count.");
		}
		if (!subtreeMap.find(Index(), &value) || value != 5)
		{
			return PYXIS__ASSERT___FAIL(
				"Wrong contents.");
		}

		return true;
	}
	
	bool testInsert() const
	{
		SubtreeMap subtreeMap;
		Value value;

		// Insert a multi-step cell 1xxx.
		if (subtreeMap.find(index10401))
		{
			return PYXIS__ASSERT___FAIL(
				"The uninserted leaf was found.");
		}
		subtreeMap.insert(index10401); // Result: 10401
		if (!subtreeMap.find(index10401))
		{
			return PYXIS__ASSERT___FAIL(
				"The inserted leaf was not found.");
		}

		// Insert branch at end of trunk; consolidate.
		if (subtreeMap.find(index10400))
		{
			return PYXIS__ASSERT___FAIL(
				"The uninserted leaf was found.");
		}
		subtreeMap.insert(index10400); // Result: 104
		Level stepCount;
		if (!subtreeMap.find(index104, &value, &stepCount) ||
			stepCount.offset != index104.getStepCount().offset)
		{
			return PYXIS__ASSERT___FAIL(
				"The consolidated parent should be in the tree.");
		}
		if (subtreeMap.find(index10401, &value, &stepCount))
		{
			if (stepCount.offset != index104.getStepCount().offset)
			{
				return PYXIS__ASSERT___FAIL(
					"The consolidated children should no longer be explicitly in the tree.");
			}
		} else 
		{
			return PYXIS__ASSERT___FAIL(
				"The consolidated children should be implicitly in the tree.");
		}
		if (subtreeMap.find(index10400, &value, &stepCount))
		{
			if (stepCount.offset != index104.getStepCount().offset)
			{
				return PYXIS__ASSERT___FAIL(
					"The consolidated children should no longer be explicitly in the tree.");
			}
		} else 
		{
			return PYXIS__ASSERT___FAIL(
				"The consolidated children should be implicitly in the tree.");
		}

		// Insert same again; should be no-op.
		subtreeMap.insert(index104); // Result: 104
		if (!subtreeMap.find(index104))
		{
			return PYXIS__ASSERT___FAIL(
				"The consolidated parent was not found.");
		}
		if (!subtreeMap.find(index10401))
		{
			return PYXIS__ASSERT___FAIL(
				"The consolidated children should be implicitly in the tree.");
		}
		if (!subtreeMap.find(index10400))
		{
			return PYXIS__ASSERT___FAIL(
				"The consolidated children should be implicitly in the tree.");
		}

		// Insert branch at some branch point within trunk.
		if (subtreeMap.find(index005))
		{
			return PYXIS__ASSERT___FAIL(
				"The uninserted leaf was found.");
		}
		subtreeMap.insert(index005); // Result: 005, 104
		if (!subtreeMap.find(index005))
		{
			return PYXIS__ASSERT___FAIL(
				"The inserted leaf was not found.");
		}
		if (!subtreeMap.find(index104))
		{
			return PYXIS__ASSERT___FAIL(
				"The original leaf was not found.");
		}
		if (2 != subtreeMap.getPairs().getCount())
		{
			return PYXIS__ASSERT___FAIL(
				"The leaf count is incorrect.");
		}

		{
			// Test consolidation at branch point:
			// add all other siblings, and make sure it doesn't consolidate.
			// Insert leaf at branch point, and make sure it consolidates.
			subtreeMap.insert(index001); // Result: 001, 005, 104
			if (!subtreeMap.find(index001))
			{
				return PYXIS__ASSERT___FAIL(
					"The inserted leaf was not found.");
			}
			subtreeMap.insert(index002); // Result: 001, 002, 005, 104
			if (!subtreeMap.find(index002))
			{
				return PYXIS__ASSERT___FAIL(
					"The inserted leaf was not found.");
			}
			subtreeMap.insert(index003); // Result: 001, 002, 003, 005, 104
			if (!subtreeMap.find(index003))
			{
				return PYXIS__ASSERT___FAIL(
					"The inserted leaf was not found.");
			}
			subtreeMap.insert(index004); // Result: 001, 002, 003, 004, 005, 104
			if (!subtreeMap.find(index004))
			{
				return PYXIS__ASSERT___FAIL(
					"The inserted leaf was not found.");
			}
			if (6 != subtreeMap.getPairs().getCount())
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf count is incorrect.");
			}
			subtreeMap.insert(index000); // Result: 0, 104
			{
				Index index0;
				index0.stepToChild(0);
				Value value;
				Level stepCount;
				if (!subtreeMap.find(index0, &value, &stepCount) || stepCount.offset != index0.getStepCount().offset)
				{
					return PYXIS__ASSERT___FAIL(
						"The inserted leaf was not found.");
				}
			}
			if (2 != subtreeMap.getPairs().getCount())
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf count is incorrect.");
			}
		}

		// Insert parent, and ensure that consolidation to root occurs.
		subtreeMap.insert(index10); // Result: {}
		if (!subtreeMap.getIsFull())
		{
			return PYXIS__ASSERT___FAIL(
				"The tree should be empty.");
		}

		return true;
	}


	bool testRemove() const
	{
		{
			SubtreeMap subtreeMap;

			// Insert root.
			subtreeMap.insert(index);
			if (!subtreeMap.find(index))
			{
				return PYXIS__ASSERT___FAIL(
					"The root was not found.");
			}

			// Remove implicit; verify that siblings are created.
			subtreeMap.remove(index10401); // Result: {}, 0, 100, 101, 102, 103, 105, 10400
			if (subtreeMap.find(index))
			{
				return PYXIS__ASSERT___FAIL(
					"The root index should not be present.");
			}
			if (!subtreeMap.find(index0))
			{
				return PYXIS__ASSERT___FAIL(
					"The sibling was not found.");
			}
			if (subtreeMap.find(index1))
			{
				return PYXIS__ASSERT___FAIL(
					"The ancestor should not be present.");
			}
			if (subtreeMap.find(index10))
			{
				return PYXIS__ASSERT___FAIL(
					"The ancestor should not be present.");
			}
			if (!subtreeMap.find(index100))
			{
				return PYXIS__ASSERT___FAIL(
					"The sibling was not found.");
			}
			if (!subtreeMap.find(index101))
			{
				return PYXIS__ASSERT___FAIL(
					"The sibling was not found.");
			}
			if (!subtreeMap.find(index102))
			{
				return PYXIS__ASSERT___FAIL(
					"The sibling was not found.");
			}
			if (!subtreeMap.find(index103))
			{
				return PYXIS__ASSERT___FAIL(
					"The sibling was not found.");
			}
			if (subtreeMap.find(index104))
			{
				return PYXIS__ASSERT___FAIL(
					"The ancestor should not be present.");
			}
			if (!subtreeMap.find(index105))
			{
				return PYXIS__ASSERT___FAIL(
					"The sibling was not found.");
			}
			if (subtreeMap.find(index1040))
			{
				return PYXIS__ASSERT___FAIL(
					"The ancestor should not be present.");
			}
			if (!subtreeMap.find(index10400))
			{
				return PYXIS__ASSERT___FAIL(
					"The sibling leaf was not found.");
			}
			if (subtreeMap.find(index10401))
			{
				return PYXIS__ASSERT___FAIL(
					"The removed index should not be present.");
			}

			// Insert the remove one; make sure it collapses to root.
			subtreeMap.insert(index10401); // Result: {}
			if (!subtreeMap.getIsFull())
			{
				return PYXIS__ASSERT___FAIL(
					"The subtree map did not consolidate properly.");
			}

			// Insert implicitly contained and verify no-op.
			subtreeMap.insert(index10400); // Result: {}
			if (!subtreeMap.find(index10400))
			{
				return PYXIS__ASSERT___FAIL(
					"The inserted index was not found.");
			}
			if (!subtreeMap.getIsFull())
			{
				return PYXIS__ASSERT___FAIL(
					"The subtree map should still be full; inserting implicitly contained should be no-op.");
			}

			// Remove root and insert 10400; verify it's there explicitly.
			subtreeMap.remove(index); // Result: empty
			if (!subtreeMap.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL(
					"Removing the root should result in an empty tree.");
			}
			subtreeMap.insert(index10400);
			{
				Value value;
				Level stepCount;
				if (!subtreeMap.find(index10400, &value, &stepCount) || stepCount.offset != index10400.getStepCount().offset)
				{
					return PYXIS__ASSERT___FAIL(
						"The explicitly inserted index was not found.");
				}
			}

			// Try to remove something not present (argument ends in the middle of a trunk).
			if (subtreeMap.find(index1040))
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should not be present.");
			}
			subtreeMap.remove(index1040); // Result: 10400
			if (subtreeMap.find(index1040))
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should not be present.");
			}
			if (!subtreeMap.find(index10400))
			{
				return PYXIS__ASSERT___FAIL(
					"The original leaf was not found.");
			}

			// Try to remove something not present (argument and trunk differ).
			if (subtreeMap.find(index1020))
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should not be present.");
			}
			subtreeMap.remove(index1020); // Result: 10400
			if (subtreeMap.find(index1020))
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should not be present.");
			}
			if (!subtreeMap.find(index10400))
			{
				return PYXIS__ASSERT___FAIL(
					"The original leaf was not found.");
			}

			// Try to remove something not present (branch not present, but others are).
			subtreeMap.remove(index10401);
			if (!subtreeMap.find(index10400))
			{
				return PYXIS__ASSERT___FAIL(
					"The original leaf was not found.");
			}

			// Remove an implicit descendant, and verify that the siblings are created.
			subtreeMap.remove(index104000); // Result: 104001, 104002, 104003, 104004, 104005
			size_t leafCount = index104000.getChildCount() - 1;
			if (subtreeMap.getPairs().getCount() != leafCount)
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf count is incorrect.");
			}
			if (subtreeMap.find(index104000))
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should not be present.");
			}
			if (!subtreeMap.find(index104001)) 
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should be present.");
			}
			if (!subtreeMap.find(index104002)) 
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should be present.");
			}
			if (!subtreeMap.find(index104003)) 
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should be present.");
			}
			if (!subtreeMap.find(index104004)) 
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should be present.");
			}
			if (!subtreeMap.find(index104005)) 
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should be present.");
			}

			// Remove the same one again; verify no change.
			subtreeMap.remove(index104000); // Result: 104001, 104002, 104003, 104004, 104005
			if (subtreeMap.find(index104000))
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should not be present.");
			}
			if (subtreeMap.getPairs().getCount() != leafCount)
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf count is incorrect.");
			}

			// Remove one exactly.
			subtreeMap.remove(index104001); // Result: 104002, 104003, 104004, 104005
			--leafCount;
			if (subtreeMap.getPairs().getCount() != leafCount)
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf count is incorrect.");
			}
			if (subtreeMap.find(index104001))
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should not be present.");
			}
			if (!subtreeMap.find(index104002)) 
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should be present.");
			}
			if (!subtreeMap.find(index104003)) 
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should be present.");
			}
			if (!subtreeMap.find(index104004)) 
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should be present.");
			}
			if (!subtreeMap.find(index104005)) 
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should be present.");
			}

			// Insert it again.
			subtreeMap.insert(index104001); // Result: 104001, 104002, 104003, 104004, 104005
			++leafCount;
			if (subtreeMap.getPairs().getCount() != leafCount)
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf count is incorrect.");
			}

			// Create two branches at a node, then remove one by one and ensure
			// that unbranching occurs properly.
			subtreeMap.insert(index1020); // Result: 102, 104001, 104002, 104003, 104004, 104005
			++leafCount;
			if (subtreeMap.getPairs().getCount() != leafCount)
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf count is incorrect.");
			}
			if (!subtreeMap.find(index1020))
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should be present.");
			}
			subtreeMap.insert(index1030); // Result: 102, 103, 104001, 104002, 104003, 104004, 104005
			++leafCount;
			if (subtreeMap.getPairs().getCount() != leafCount)
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf count is incorrect.");
			}
			if (!subtreeMap.find(index1030))
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should be present.");
			}
			subtreeMap.remove(index104001); // Result: 102, 103, 104002, 104003, 104004, 104005
			--leafCount;
			if (subtreeMap.getPairs().getCount() != leafCount)
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf count is incorrect.");
			}
			if (subtreeMap.find(index104001))
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should not be present.");
			}
			subtreeMap.remove(index1030); // Result: 102, 104002, 104003, 104004, 104005
			--leafCount;
			if (subtreeMap.getPairs().getCount() != leafCount)
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf count is incorrect.");
			}
			if (subtreeMap.find(index1030))
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should not be present.");
			}
			subtreeMap.remove(index1020); // Result: 104002, 104003, 104004, 104005
			--leafCount;
			if (subtreeMap.getPairs().getCount() != leafCount)
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf count is incorrect.");
			}
			if (subtreeMap.find(index1020))
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should not be present.");
			}
			if (!subtreeMap.find(index104002)) 
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should be present.");
			}
			if (!subtreeMap.find(index104003)) 
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should be present.");
			}
			if (!subtreeMap.find(index104004)) 
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should be present.");
			}
			if (!subtreeMap.find(index104005)) 
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should be present.");
			}

			// Test consolidation of insertion.
			subtreeMap.insert(index104001); // Result: 104001, 104002, 104003, 104004, 104005
			++leafCount;
			if (subtreeMap.getPairs().getCount() != leafCount)
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf count is incorrect.");
			}
			if (!subtreeMap.find(index104001))
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should be present.");
			}
			subtreeMap.insert(index104000); // Result: 10400
			if (subtreeMap.getPairs().getCount() != 1)
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf count is incorrect.");
			}
			if (!subtreeMap.find(index10400))
			{
				return PYXIS__ASSERT___FAIL(
					"The leaf should be present.");
			}

			subtreeMap.remove(index1); // Result: 10400
			if (subtreeMap.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL(
					"Should not be empty.");
			}
			subtreeMap.remove(index10400); // Result: empty
			if (!subtreeMap.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL(
					"Should be empty.");
			}
		}

		{
			SubtreeMap< std::string > subtreeMap;
			std::string value;
			subtreeMap.insert(Index(), ""); // Result: {}:""
			if (subtreeMap.getIsEmpty())
			{
				return PYXIS__ASSERT___FAIL(
					"Should not be empty.");
			}
			subtreeMap.insert(index1, "1"); // Result: 0:"", 1:"1"
			if (subtreeMap.getPairs().getCount() != 2)
			{
				return PYXIS__ASSERT___FAIL(
					"Incorrect count.");
			}
			// Remove a branch whose edge matches the index.  Should be no-op.
			subtreeMap.remove(Index()); // Result: 0:"", 1:"1"
			if (subtreeMap.getPairs().getCount() != 2)
			{
				return PYXIS__ASSERT___FAIL(
					"Incorrect count.");
			}
			// Remove a child whose only sibling is a leaf; ensure correct consolidation.
			subtreeMap.remove(index0); // Result: 1:"1"
			if (subtreeMap.getPairs().getCount() != 1)
			{
				return PYXIS__ASSERT___FAIL(
					"Incorrect count.");
			}
			if (!subtreeMap.find(index1, &value) || value != "1")
			{
				return PYXIS__ASSERT___FAIL(
					"Incorrect contents.");
			}
		}

		return true;
	}

	bool testInsertMultiple() const
	{
		// TODO
		return true;
	}

	bool testRemoveMultiple() const
	{
		// TODO
		return true;
	}

	bool testWriteRead() const
	{
		// TODO
		return true;
	}

	bool testCountDescendants() const
	{
		SubtreeMap subtreeMap;

		if (subtreeMap.getDescendantCount(Level(0)) != 0)
		{
			return PYXIS__ASSERT___FAIL(
				"Incorrect descendant count.");
		}
		if (subtreeMap.getDescendantCount(Level(1)) != 0)
		{
			return PYXIS__ASSERT___FAIL(
				"Incorrect descendant count.");
		}

		subtreeMap.insert(index105);
		subtreeMap.insert(index104004);

		if (subtreeMap.getDescendantCount(Level(0)) != 0)
		{
			return PYXIS__ASSERT___FAIL(
				"Incorrect descendant count.");
		}
		if (subtreeMap.getDescendantCount(Level(1)) != 0)
		{
			return PYXIS__ASSERT___FAIL(
				"Incorrect descendant count.");
		}
		if (subtreeMap.getDescendantCount(Level(2)) != 0)
		{
			return PYXIS__ASSERT___FAIL(
				"Incorrect descendant count.");
		}
		if (subtreeMap.getDescendantCount(Level(3)) != 1)
		{
			return PYXIS__ASSERT___FAIL(
				"Incorrect descendant count.");
		}
		if (subtreeMap.getDescendantCount(Level(4)) != 1)
		{
			return PYXIS__ASSERT___FAIL(
				"Incorrect descendant count.");
		}
		if (subtreeMap.getDescendantCount(Level(5)) != 2)
		{
			return PYXIS__ASSERT___FAIL(
				"Incorrect descendant count.");
		}

		return true;
	}

public:
	explicit Test() :
	index(),
	index0("0"),
	index00("00"),
	index000("000"),
	index001("001"),
	index002("002"),
	index003("003"),
	index004("004"),
	index005("005"),
	index0010("0010"),
	index1("1"),
	index10("10"),
	index100("100"),
	index101("101"),
	index102("102"),
	index103("103"),
	index104("104"),
	index105("105"),
	index1020("1020"),
	index1030("1030"),
	index1040("1040"),
	index10400("10400"),
	index10401("10401"),
	index104000("104000"),
	index104001("104001"),
	index104002("104002"),
	index104003("104003"),
	index104004("104004"),
	index104005("104005")
	{}

	operator bool() const
	{
		return (
			testDefaultConstruction() &&
			testInsertValue() &&
			testWriteRead() &&
			testInsert() &&
			testInsertMultiple() &&
			testRemove() &&
			testRemoveMultiple() &&
			testCountDescendants());
	}
};

#endif
