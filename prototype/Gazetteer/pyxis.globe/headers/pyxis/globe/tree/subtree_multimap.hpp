#if !defined(PYXIS__GLOBE__TREE__SUBTREE_MULTIMAP)
#define PYXIS__GLOBE__TREE__SUBTREE_MULTIMAP

#include "pyxis/compact_vector.hpp"
#include "pyxis/compact_set.hpp"
#include "pyxis/globe/tree/subtree_set.hpp"
#include "pyxis/multimap_interface.hpp"
#include "pyxis/set.hpp"
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>

namespace Pyxis
{
	namespace Globe
	{
		namespace Tree
		{
			template < typename Value > class SubtreeMultimap;
		}
	}
}

#define PYXIS__GLOBE__TREE__SUBTREE_MULTIMAP__REFACTORED 1
#if PYXIS__GLOBE__TREE__SUBTREE_MULTIMAP__REFACTORED

// TODO: Make this a specialization of Multimap< Subtree const &, Value >,
// once Subtree and Multimap are implemented.
template < typename Value >
class Pyxis::Globe::Tree::SubtreeMultimap :
public MultimapInterface< Index const &, Value >
{

#define PYXIS__GLOBE__TREE__SUBTREE_MULTIMAP__RADIX_TREE 1
#if PYXIS__GLOBE__TREE__SUBTREE_MULTIMAP__RADIX_TREE

	/*
	A node represents a subtree.

	A node has the following properties:
	-	The index, which is the most consolidated index that describes the subtree.
		Is never of the form x0v0, where 'v' is non-zero,
		because it is the only child of x0v and can be consolidated.
	-	The child count, which is the count of child nodes and is always greater than 1.
		If the index has more than 1 child, this is the node child count.
		Otherwise, the index is stepped to the single child until the index has a child count > 1;
		this is the node child count.

	This is a private, unsafe data structure;
	the correct index and/or child count for the node must always be provided.
	This is not checked.
	*/
	class Node :
	boost::noncopyable // Has special copy constructor and assign method that additionally take Index.
	{	
		// The descendant nodes.
		class Descendants : boost::noncopyable
		{
			typedef CompactVector< Step > Trunk;

			class Branches : boost::noncopyable
			{
#if !NDEBUG
				char count;
#endif

				boost::scoped_array< Node > nodes;

			public:

				Branches() :
#if !NDEBUG
				count(),
#endif
				nodes()
				{}

				Node & operator [](Step const step)
				{
#if !NDEBUG
					assert(step < count);
#endif

					return this->nodes[step];
				}

				Node const & operator [](Step const step) const
				{
#if !NDEBUG
					assert(step < count);
#endif

					return this->nodes[step];
				}

				void swap(Branches & branches)
				{
#if !NDEBUG
					std::swap(this->count, branches.count);
#endif
					this->nodes.swap(branches.nodes);
				}

				bool getIsEmpty() const
				{
					return !this->nodes;
				}

				void setIsEmpty()
				{
#if !NDEBUG
					this->count = 0;
#endif
					this->nodes.reset();
				}

				void reset(char count)
				{
#if !NDEBUG
					this->count = count;
#endif
					this->nodes.reset(new Node[count]);
				}
			};
			
			// Step index to child and return true, or return false if at end.
			static bool stepToChild(
				Index const & index, char & stepCount)
			{
				assert(stepCount <= index.getStepCount().offset &&
					"The step count is out of range.");

				if (stepCount == index.getStepCount().offset) { return false; }
				++stepCount;
				return true;
			}
			
			// If the child count is one, step to the single child.
			// Postcondition: index.getChildCount() > 1
			static void tryStepToSingleChild(
				Index & index)
			{
				assert(0 < index.getChildCount());
				if (1 == index.getChildCount())
				{
					index.stepToChild();
					assert(1 < index.getChildCount());
				}
			}

			// If there is a single child, step to that child.  Otherwise, no-op.
			// Returns false if runs out of steps to do its thing.
			static bool tryStepToSingleChild(
				Index const & index, char & stepCount)
			{
				assert(stepCount <= index.getStepCount().offset &&
					"The step count is out of range.");
				
				if (1 == index.getChildCount(Level(stepCount)) &&
					!stepToChild(index, stepCount))
				{
					return false;
				}
				assert(1 < index.getChildCount(Level(stepCount)));
				return true;
			}
			
			// Steps to the child node and returns true, or returns false
			// if there wasn't one (and leaves stepCount unchanged).
			static bool stepToChildNode(
				Index const & index, char & stepCount)
			{
				assert(stepCount <= index.getStepCount().offset &&
					"The step count is out of range.");
				
				char newStepCount = stepCount;
				if (tryStepToSingleChild(index, newStepCount) && stepToChild(index, newStepCount))
				{
					stepCount = newStepCount;
					return true;
				}
				return false;
			}

			// The values mapped to a descendant.
			CompactSet< Value > values;

			// The single-child steps that descend from this node.
			Trunk trunk;

			// The child nodes at the end of the trunk.
			Branches branches;

		private:

			// Helper.
			// Descends the index to that of the parent node of the branches.
			void descend(
				Index & index /* The node index */) const
			{
				// Descend to the parent node of the branches.
				for (typename Trunk::Elements elements(this->trunk);
					!elements.getIsEmpty(); elements.popFront())
				{
					tryStepToSingleChild(index);
					index.stepToChild(elements.getFront());
				}
			}

			// Splits the trunk at the given point.
			// Does not modify descendant value set.
			void split(
				char trunkStepCount /* The new trunk step count. */,
				char branchCount /* The new branch count. */)
			{
				assert((size_t)trunkStepCount < this->trunk.getCount());
				assert(1 < branchCount);
			
				// Create a new node.  This will be swapped into a child.
				Node node;

				// Copy the descendant values into the node trunk.
				node.descendants.values = this->values;

				// Get the branch step.
				Step const branchStep = this->trunk[trunkStepCount];

				// Move the end of this trunk to the node trunk.
				node.descendants.trunk.append(
					&this->trunk[0] + trunkStepCount + 1, // Skip branch step
					&this->trunk[0] + this->trunk.getCount());
				this->trunk.truncate(this->trunk.getCount() - trunkStepCount);

				// Swap the branches.
				node.descendants.branches.swap(this->branches);

				// Create new branches here.
				assert(this->branches.getIsEmpty());
				this->branches.reset(branchCount);

				// Swap the node with the right branch.
				this->branches[branchStep].swap(node);
			}
			
			// Merges the branch with the trunk.
			// Does not modify descendant value set.
			// Updates the index to refer to the new branch parent node.
			void merge(
				Index & index /* The index of the branch parent node. */,
				Step branchStep /* The branch step to merge. */)
			{
				assert(!this->branches.getIsEmpty());

				// Get the branch.
				Descendants & branch = this->branches[branchStep].descendants;
				
				// Append the branch step to the trunk, and update the index.
				this->trunk.append(&branchStep, &branchStep + 1);
				tryStepToSingleChild(index);
				index.stepToChild(branchStep);

				// If the branch trunk is not empty,
				// append the branch trunk to the trunk, and update the index.
				if (!branch.trunk.getIsEmpty())
				{
					this->trunk.append(
						&branch.trunk[0],
						&branch.trunk[0] + branch.trunk.getCount());
					branch.descend(index);
				}

				// Swap the branches out of the branch into a temporary.
				// Need to use a temporary to avoid a self-double-delete.
				// Postcondition: branch.branches is empty.
				Branches temporaryBranches;
				temporaryBranches.swap(branch.branches);

				// Swap the branches into this node.
				// Postcondition: temporary branches has original branches.
				this->branches.swap(temporaryBranches);
			}

			// Helper.
			// Gets the branch count.
			// Returns 0 if not enough steps in index.
			char getBranchCount(
				Index const & index, char stepCount /* The index of the last node before the branches. */) const
			{
				if (!tryStepToSingleChild(index, stepCount)) { return 0; }
				char const branchCount = index.getChildCount(Level(stepCount));
				assert(1 < branchCount);
				return branchCount;
			}
			
			// Helper.
			// Gets the branch count.
			// Doesn't roll up the index afterward.
			char getBranchCount(
				Index & index /* The index of the last node before the branches. */) const
			{
				tryStepToSingleChild(index);
				char branchCount = index.getChildCount();
				assert(1 < branchCount);
				return branchCount;
			}

			// Doesn't roll up the index afterward.
			void findInBranches(Value value, SubtreeSet & results,
				Index & index /* The index of the last node before the branches. */) const
			{
				assert(!this->branches.getIsEmpty());
				assert(this->values.find(value));

				// Bypass the single child to get to the children.
				tryStepToSingleChild(index);

				// Find the value mappings in each.
				{
					char branchCount = index.getChildCount();
					assert(1 < branchCount);
					do 
					{
						Step const branchStep = --branchCount;
						index.stepToChild(branchStep);
						this->branches[branchStep].find(value, results, index);
						index.stepToParent();
					} while (branchCount);
				}
			}

			// Helper.
			// Removes the value from all branches and descendants thereof.
			// Updates 'values'.
			// If the result is empty branches, clears the tree.
			// Doesn't roll up the index afterward.
			void removeFromBranches(
				Value value,
				Index & index /* The index of the last node before the branches. */)
			{
				assert(!this->branches.getIsEmpty());
				assert(this->values.find(value));

				// Bypass the single child to get to the children.
				tryStepToSingleChild(index);

				// Remove the value from each, and track whether each branch ends up empty.
				bool areBranchesEmpty = true;
				{
					char branchCount = index.getChildCount();
					assert(1 < branchCount);
					do 
					{
						Step const branchStep = --branchCount;
						index.stepToChild(branchStep);
						Node & branch = this->branches[branchStep];
						branch.remove(value, index);
						areBranchesEmpty = areBranchesEmpty && branch.getIsEmpty();
						index.stepToParent();
					} while (branchCount);
				}

				// If the branches are all empty, set the descendants to empty.
				if (areBranchesEmpty)
				{
					setIsEmpty();
				}
				
				// Remove the value from the descendant value cache.
				this->values.remove(value);
			}

		public:
		
			Descendants() : values(), trunk(), branches() {}

			Descendants(Descendants const & that,
				Index & index /* The node index; final value is unchanged. */) :
			values(that.values),
			trunk(that.trunk),
			branches()
			{
				if (!that.branches.getIsEmpty())
				{
					// Capture the step count before descending.
					char const stepCount = index.getStepCount().offset;

					// Descend to the parent node of the branches.
					descend(index);
					
					// Get the branch count.
					char branchCount = getBranchCount(index);

					// Construct the branches array.
					this->branches.reset(branchCount);

					// Copy the branches.
					do
					{
						index.stepToChild(--branchCount);
						this->branches[branchCount].assign(that.branches[branchCount], index);
						index.stepToParent();
					} while (branchCount);

					// Roll back up.
					index.stepToAncestor(Level(index.getStepCount().offset - stepCount));
				}
			}
			
			// Swap the contents of the node.
			void swap(Descendants & descendants)
			{
				values.swap(descendants.values);
				trunk.swap(descendants.trunk);
				branches.swap(descendants.branches);
			}

			// Assign the descendants.
			void assign(Descendants const & descendants,
				Index & index /* The node index; final value is unchanged */)
			{
				Descendants(descendants, index).swap(*this);
			}
			
			// Returns true if empty.	
			bool getIsEmpty() const
			{
				assert(!values.getIsEmpty() || (trunk.getIsEmpty() && branches.getIsEmpty()));
				return values.getIsEmpty();
			}

			// Empties the collection.
			void setIsEmpty()
			{
				values.setIsEmpty();
				trunk.setIsEmpty();
				branches.setIsEmpty();
			}
			
			// Writes a user-friendly string describing the structure; for diagnostics.
			void write(std::ostream & output,
				Index & index /* The node index; final value is unchanged. */) const
			{
				// Get the step count for the node index.
				char const stepCount = index.getStepCount().offset;

				// Calculate indent for each step.
				std::string indent(stepCount, ' ');

				// Write values mapped to descendant(s).
				output << this->values;

				// If this is not a leaf, write.
				if (!this->branches.getIsEmpty())
				{
					// Write trunk.
					for (typename Trunk::Elements elements(this->trunk); ; elements.popFront())
					{
						// If it has only 1 child, advance and write the child.
						if (1 == index.getChildCount())
						{
							index.stepToChild();
							indent.push_back(' ');
							output << std::endl << indent << "(0)";
						}
						
						// If the end of the trunk is reached, break.
						if (elements.getIsEmpty()) { break; }

						// Write the next step in the trunk.
						{
							int const step = elements.getFront();
							index.stepToChild(step);
							indent.push_back(' ');
							output << std::endl << indent << "[" << step << "]";
						}
					}
					
					// Get the branch count.
					char branchCount = index.getChildCount();
					assert(1 < branchCount);

					// Write the branches.
					indent.push_back(' ');
					int branchStep = 0;
					do
					{
						index.stepToChild(branchStep);
						output << std::endl << indent << "[" << branchStep << "]";
						this->branches[branchStep].write(output, index);
						index.stepToParent();
					} while (++branchStep < branchCount);
				}

				// Roll back up.
				index.stepToAncestor(Level(index.getStepCount().offset - stepCount));
			}

			// Removes the value.
			// Removes from values, then removes from each descendant.
			void remove(Value value, 
				Index & index /* The node index; the final value is unchanged */)
			{
				if (this->values.find(value))
				{
					assert(!this->branches.getIsEmpty());

					char const stepCount = index.getStepCount().offset;
				
					descend(index);
					removeFromBranches(value, index);

					// Roll back up.
					index.stepToAncestor(Level(index.getStepCount().offset - stepCount));
				}
			}

			// Returns true if the value is found.
			bool find(Value value) const
			{
				return this->values.find(value);
			}

			// Finds the subtrees that the value is mapped to.
			SubtreeSet & find(Value value, SubtreeSet & results,
				Index & index /* The node index; final value is unchanged. */) const
			{
				if (this->values.find(value))
				{
					assert(!this->branches.getIsEmpty());
					
					char const stepCount = index.getStepCount().offset;
					
					descend(index);
					findInBranches(value, results, index);			

					// Roll back up.
					index.stepToAncestor(Level(index.getStepCount().offset - stepCount));
				}
				return results;
			}

			// Returns true if all values mapped to the index are visited, and false
			// if cancelled by callback.
			bool visit(
				Index const & index, char stepCount, // The index up to step count is that of this node.
				MutableSetInterface< Value > & results, 
				FunctorInterface< bool, Value > & callback,
				bool includePartial) const
			{
				assert(stepCount <= index.getStepCount().offset &&
					"The step count is out of range.");

				if (!getIsEmpty())
				{
					assert(!this->branches.getIsEmpty());

					// Try to advance the index to the next node.
					// If we can't, the index refers to this node.
					if (stepToChildNode(index, stepCount))
					{
						assert(0 < stepCount);

						// Descend the trunk.
						// If we run out of steps in the index, 
						// it is an ancestor of the branches; fall through to partials.
						typename Trunk::Elements elements(this->trunk);
						do
						{
							// Get the index step.
							Step const step = index.getSteps()[stepCount - 1];

							// If the trunk has been iterated, visit the correct branch and return.
							if (elements.getIsEmpty())
							{
								return this->branches[step].visit(
									index, stepCount, results, callback, includePartial);
							}

							// Advance to the next step in the trunk.
							Step const trunkStep = elements.getFront();
							elements.popFront();

							// If it differs from the index step, the index isn't contained;
							// return true.
							if (step != trunkStep) { return true; }

						} while (stepToChildNode(index, stepCount));
					}
					
					if (includePartial)
					{
						return this->values.visit(callback, results);
					}
				}
				return true;
			}

			// Returns true if the value is to be inserted into the full tree.
			// Populates collapsed value set with values that have been
			// collapsed across the descendants.
			bool insert(
				Index const & index, char stepCount, // The index up to step count is that of this node.
				Value value,
				MutableSetInterface< Value > & collapsedValues)
			{
				assert(stepCount <= index.getStepCount().offset &&
					"The step count is out of range.");

				// Capture step count before descent.
				char const initialStepCount = stepCount;
				
				// Try to advance the index to the next node.
				// If we can't, the index refers to this node.
				if (stepToChildNode(index, stepCount))
				{
					assert(0 < stepCount);

					char parentStepCount = initialStepCount;

					// If this is a leaf, insert as a trunk with a full branch.
					if (this->branches.getIsEmpty())
					{
						assert(this->trunk.getIsEmpty());

						// Walk down node indices, down to (not including) last node index,
						// and get steps for trunk.
						std::vector< Step > trunkSteps;
						for (; ; trunkSteps.push_back(index.getSteps()[parentStepCount - 1]))
						{
							// Try to step to the next node index.
							// If no more, this one is the branch.
							char childStepCount = stepCount;
							if (!stepToChildNode(index, childStepCount))
							{
								// Push the trunk.
								{
									size_t const trunkStepsSize = trunkSteps.size();
									if (trunkStepsSize)
									{
										Step const * const begin = &trunkSteps[0];
										trunk.append(begin, begin + trunkStepsSize);
									}
								}

								// Get the branch count.
 								char const branchCount = getBranchCount(index, parentStepCount);
								assert(1 < branchCount);
								
								// Construct branches.
								this->branches.reset(branchCount);
							
								// Get branch step.
								char const branchStep = index.getSteps()[stepCount - 1];

								// Set the correct branch to full for value.
								this->branches[branchStep].values.insert(value);
								
								// The value is not mapped to this entire node.
								this->values.insert(value);
								return false;
							}
							parentStepCount = stepCount;
							stepCount = childStepCount;
						}
					}
				
					// Descend the trunk and split either at difference, or if index runs out.
					// Postconditions: parentStepCount is branch parent node; stepCount is branch node.
					boost::optional< char > grandparentStepCount;
					size_t trunkStepCount = trunk.getCount();
					assert(trunkStepCount <= boost::integer_traits< char >::const_max + 1);
					for (size_t trunkStepOffset = 0; trunkStepOffset < trunkStepCount; ++trunkStepOffset)
					{
						Step const step = index.getSteps()[stepCount - 1];
						Step const trunkStep = this->trunk[trunkStepOffset];
						char childStepCount = stepCount;
						if (step != trunkStep || !stepToChildNode(index, childStepCount))
						{
							// Split trunk before difference.
							// Index and trunk offset are that of branch.
							split(trunkStepOffset, getBranchCount(index, parentStepCount));
							trunkStepCount = trunkStepOffset;

							// Set step count to refer to the new branch.
							stepCount = childStepCount;

							// Break so that we continue on to the correct child.
							break;
						}

						grandparentStepCount = parentStepCount;
						parentStepCount = stepCount;
						stepCount = childStepCount;
					}

					// This was checked above.
					assert(!this->branches.getIsEmpty());

					// Insert into the correct branch.
					// If this returns false, there is no value collapsing opportunity; return false.
					{
						Node & branch = this->branches[index.getSteps()[stepCount - 1]];
						if (!branch.insert(index, stepCount, value))
						{
							// The value is not mapped to this entire node.
							this->values.insert(value);
							return false;
						}
					}

					// Get the branch count.
					char branchCount = getBranchCount(index, parentStepCount);
					assert(1 < branchCount);
					
					// The branch is now full of the value.
					// If each branch is not full of the value, return false.
					{
						char branchStep = branchCount;
						do
						{
							if (!this->branches[--branchStep].getIsFull(value))
							{
								// The value is not mapped to this entire node.
								this->values.insert(value);
								return false;
							}
						} while (branchStep);
					}

					// If the trunk isn't empty, split and collapse the value up to the next node.
					// Otherwise, fall through and insert the value for the entire node.
					if (0 < trunkStepCount)
					{
						// Remove the value from each branch.
						{
							char branchStep = branchCount;
							do
							{
								this->branches[--branchStep].values.remove(value);
							} while (branchStep);
						}

						// Split the branch.
						assert(grandparentStepCount);
						split(trunkStepCount - 1, getBranchCount(index, *grandparentStepCount));

						// For the correct child branch, set the value to be fully inserted.
						this->branches[index.getSteps()[parentStepCount - 1]].values.insert(value);

						// The value is not mapped to this entire node.
						this->values.insert(value);
						return false;
					}
				}

				// Insert the value for the entire node.
				Index nodeIndex(index, Level(initialStepCount));
				remove(value, nodeIndex);
				collapsedValues.insert(value);
				return true;
			}

			// Value collapsing is not possible, except by caller in above case.
			void removePartial(
				Index const & index, char stepCount, // The index up to step count is that of this node.
				Value value)
			{
				assert(stepCount <= index.getStepCount().offset &&
					"The step count is out of range.");

				// Capture step count before descent.
				char const initialStepCount = stepCount;

				// Try to advance the index to the next node.
				// If we can't, the index refers to this node.
				if (stepToChildNode(index, stepCount))
				{
					assert(0 < stepCount);

					// Get the branch count of the node index.
					char branchCount = getBranchCount(index, initialStepCount);
					assert(1 < branchCount);

					// If there is a trunk, split it.
					// Otherwise, create branches if necessary.
					if (this->trunk.getIsEmpty())
					{
						if (this->branches.getIsEmpty())
						{
							this->branches.reset(branchCount);
						}
					} else
					{
						assert(!this->branches.getIsEmpty());
						split(0, branchCount);
					}

					// Get the step.
					Step const step = index.getSteps()[stepCount - 1];

					// Fill the siblings.
					do
					{
						Step branchStep = --branchCount;
						if (branchStep != step)
						{
							// Set to full of value.
							branches[branchStep].values.insert(value);
						}
					} while (branchCount);
					
					// Recursive call on child.
					branches[step].descendants.removePartial(index, stepCount, value);

					// The value is not mapped to this entire node, but to descendant(s).
					this->values.insert(value);
				}
			}

			// Returns true if the value was removed from the entire tree.
			bool remove(
				Index const & index, char stepCount, // The index up to step count is that of this node.
				Value value,
				MutableSetInterface< Value > & collapsedValues)
			{
				assert(stepCount <= index.getStepCount().offset &&
					"The step count is out of range.");

				// Capture initial step count before descent.
				char const initialStepCount = stepCount;

				// Try to advance the index to the next node.
				// If we can't, the index refers to this node.
				{
					char parentStepCount = initialStepCount;
					if (stepToChildNode(index, stepCount))
					{
						assert(0 < stepCount);

						// Descend the trunk.
						for (typename Trunk::Elements elements(trunk); ; elements.popFront())
						{
							// If we've reached the end of the trunk:
							if (elements.getIsEmpty())
							{
								// It shouldn't have been called if this is a leaf.
								assert(!this->branches.getIsEmpty());

								// Remove from the correct branch.
								Node & branch = this->branches[index.getSteps()[stepCount - 1]];
								if (!branch.remove(index, stepCount, value))
								{
									// The values in the branch node were not modified.
									return false;
								}

								// Get the branch count for this node.
								char branchCount = getBranchCount(index, parentStepCount);
								assert(1 < branchCount);
								
								// If all the branches are empty except one,
								// and it doesn't have any node values,
								// we can merge the other branches into it.
								{
									boost::optional< Step > mergeStep;
								
									for (char branchStep = branchCount; ; )
									{
										--branchStep;
										
										Node const & branch = this->branches[branchStep];
										if (!branch.values.getIsEmpty())
										{
											// If any of the branches has a non-empty value set,
											// we cannot merge.
											break;
										}
										if (!branch.descendants.getIsEmpty())
										{
											// If the merge step has already been set, bail.
											if (mergeStep) { break; }

											// Set the merge step.
											mergeStep = branchStep;
										}
										
										// If we've reached the end, merge.
										if (!branchStep)
										{
											// There must be at least one non-empty branch;
											// otherwise, this should have already been merged.
											assert(mergeStep);

											// Merge, and update branch count.
											Index temporaryIndex(index, Level(parentStepCount));
											merge(temporaryIndex, *mergeStep);
											branchCount = getBranchCount(temporaryIndex);

											break; // TODO: Audit this code some more... not sure it's right.
										}
									}
								}
								
								// If the value is in any of the branches, return false.
								{
									Step branchStep = branchCount;
									do
									{
										--branchStep;
										if (this->branches[branchStep].find(value)) { return false; }
									} while (branchStep);
								}

								// The value is not in any of the branches.
								// Remove it from descendant values list.
								this->values.remove(value);
								
								// Tell the caller it is removed entirely.
								return true;
							}

							// If the steps don't match, the index is not present.
							Step const step = index.getSteps()[stepCount - 1];
							Step const trunkStep = elements.getFront();
							if (step != trunkStep) { return false; }

							parentStepCount = stepCount;
							if (!stepToChildNode(index, stepCount))
							{
								// The index is an ancestor of the branch nodes.
								// We need to remove the value from all descendants.
								break;
							}
						}
					}
				}

				// Remove the value from the entire node.
				Index nodeIndex(index, Level(initialStepCount));
				remove(value, nodeIndex);
				return true;
			}
		};

		// The set of values mapped to this subtree.
		CompactSet< Value > values;

		// The descendants of the node.
		Descendants descendants;

	public:

		// Constructs an empty node.
		Node() :
		values(),
		descendants()
		{}
		
		// Copies the node.
		Node(Node const & node,
			Index & index /* The node index; final value is unchanged */) :
		values(node.values),
		descendants(node.descendants, index)
		{}

		// Writes a user-friendly string describing the node structure; for diagnostics.
		void write(std::ostream & output,
			Index & index /* The node index; final value is unchanged */) const
		{
			// Write values.
			output << this->values;

			// Write descendants.
			this->descendants.write(output, index);
		}

		// Swap the contents of the node.
		void swap(Node & node)
		{
			this->values.swap(node.values);
			this->descendants.swap(node.descendants);
		}

		// Assign the node.
		void assign(Node const & node,
			Index & index /* The node index; final value is unchanged */)
		{
			Node(node, index).swap(*this);
		}

		// Returns true if empty.	
		bool getIsEmpty() const
		{
			return this->values.getIsEmpty() && this->descendants.getIsEmpty();
		}

		// Empties the collection.
		void setIsEmpty()
		{
			this->values.setIsEmpty();
			this->descendants.setIsEmpty();
		}
		
		bool getIsFull(Value value) const
		{
			return this->values.find(value);
		}
		
	public: // This node

		// Returns true if the values of this node were modified.
		bool insert(Value value,
			Index & index /* The node index; final value is unchanged */)
		{
			if (!this->values.find(value))
			{
				remove(value, index);
				this->values.insert(value);
				return true;
			}
			return false;
		}

		// Remove the value from this subtree and all descendants.
		// Returns true if the values of this node were modified.
		bool remove(Value value,
			Index & index /* The node index; final value is unchanged */)
		{
			if (this->values.find(value))
			{
				assert(!this->descendants.find(value) &&
					"A value mapped to the subtree should not be explicitly mapped to any descendant.");
				this->values.remove(value);
				return true;
			}
			this->descendants.remove(value, index);
			return false;
		}
		
		// For each value mapped to this subtree:
		// if it is not in results, insert it and call callback.
		// May visit all partials.
		bool visit(
			MutableSetInterface< Value > & results, 
			FunctorInterface< bool, Value > & callback,
			bool includePartial) const
		{
			return this->values.visit(callback, results) && (
				!includePartial || this->descendants.visit(results, callback));
		}

		// Returns true if the value is mapped to this subtree
		// or one of its descendants.
		bool find(Value value) const
		{
			return this->values.find(value) || this->descendants.find(value);
		}

		// Finds the subtrees explicitly mapped to this value.
		// The 'subtreeIndex' argument is the index of this node.
		// Returns a reference to the results.
		SubtreeSet & find(Value value, SubtreeSet & results,
			Index & index /* The node index; final value is unchanged. */) const
		{
			if (this->values.find(value))
			{
				results.insert(index);
				return results;
			}
			return this->descendants.find(value, results, index);
		}

	public: // Non-strict descendant node

		// For each value mapped to the given descendant subtree,
		// or a descendant thereof:
		// if it is not in results, insert it and call callback.
		bool visit(
			Index const & index, char stepCount, // The index up to step count is that of this node.
			MutableSetInterface< Value > & results, 
			FunctorInterface< bool, Value > & callback,
			bool includePartial) const
		{
			assert(stepCount <= index.getStepCount().offset &&
				"The step count is out of range.");

			// Visit values mapped to the whole tree,
			// then visit descendants if visitation was not cancelled.
			return (this->values.visit(callback, results) && 
				this->descendants.visit(index, stepCount, results, callback, includePartial));
		}

		// Inserts the mapping of the value to the given descendant subtree.
		// Returns true if the value is now mapped to the entire tree (and wasn't before).
		bool insert(
			Index const & index, char stepCount, // The index up to step count is that of this node.
			Value value)
		{
			assert(stepCount <= index.getStepCount().offset &&
				"The step count is out of range.");

			// If it is already mapped to the entire tree, return false.
			if (this->values.find(value)) { return false; }
			
			// Insert into descendants.
			// If the tree is modified such that the value is fully inserted,
			// insert collapsed values into values and return true.
			Set< Value > collapsedValues;
			if (this->descendants.insert(index, stepCount, value, collapsedValues))
			{
				for (typename Set< Value >::Elements elements(collapsedValues);
					!elements.getIsEmpty(); elements.popFront())
				{
					this->values.insert(elements.getFront());
				}
				return true;
			}
			return false;
		}

		// Removes the mapping of the value from the given descendant subtree.
		// Returns true if the value is no longer mapped to the entire tree.
		bool remove(
			Index const & index, char stepCount, // The index up to step count is that of this node.
			Value value)
		{
			assert(stepCount <= index.getStepCount().offset &&
				"The step count is out of range.");

			// If it is already mapped to the entire tree, we need to
			// remove it and insert siblings all the way down.
			if (this->values.find(value))
			{
				// Remove the value from the tree.
				this->values.remove(value);

				// Tell the descendants to "remove partial",
				// meaning insert the siblings all the way down.
				this->descendants.removePartial(index, stepCount, value);
				return true;
			}

			// Remove from descendants. 
			// If the the tree is modified such that the value is fully removed,
			// insert collapsed values into values and return true.
			Set< Value > collapsedValues;
			if (this->descendants.remove(index, stepCount, value, collapsedValues))
			{
				for (typename Set< Value >::Elements elements(collapsedValues);
					!elements.getIsEmpty(); elements.popFront())
				{
					this->values.insert(elements.getFront());
				}
				return true;
			}
			return false;
		}

		// Populates the results with all values mapped to the given descendant subtree
		// (and its descendants, if specified),
		// and returns a reference to the results.
		MutableSetInterface< Value > & find(
			Index const & index, char stepCount, // The index up to step count is that of this node.
			MutableSetInterface< Value > & results,
			bool includePartial) const
		{
			struct : FunctorInterface< bool, Value >
			{
				bool operator ()(Value value) { return true; }
			} callback;
			visit(index, stepCount, results, callback, includePartial);
			return results;
		}

		// Returns true if the value is mapped to the given descendant subtree
		// (or a descendant thereof, if specified).
		bool find(
			Index const & index, char stepCount, // The index up to step count is that of this node.
			Value value,
			bool includePartial) const
		{
			// Use the visitor; stops (and returns false) if found.
			Set< Value > results;
			struct Callback : FunctorInterface< bool, Value >
			{
				Value value;
				explicit Callback(Value value) : value(value) {}
				bool operator ()(Value visitee) { return (visitee != value); }
			} callback(value);
			return !visit(index, stepCount, results, callback, includePartial);
		}

		// Returns true if there is at least one value mapped to the given descendant subtree
		// (or a descendant thereof, if specified).
		bool find(
			Index const & index, char stepCount,
			bool includePartial) const // The index up to step count is that of this node.
		{
			// Use the visitor; stops (and returns false) if found.
			Set< Value > results;
			struct : FunctorInterface< bool, Value >
			{
				bool operator ()(Value visitee) { return false; }
			} callback;
			return !visit(index, stepCount, results, callback, includePartial);
		}

	} node;	

#else //////////////

	/*
	A node represents a subtree.

	A node has the following properties:
	-	The index, which is the most consolidated index that describes the subtree.
		Is never of the form x0v0, where 'v' is non-zero,
		because it is the only child of x0v and can be consolidated.
	-	The child count, which is the count of child nodes and is always greater than 1.
		If the index has more than 1 child, this is the node child count.
		Otherwise, the index is stepped to the single child until the index has a child count > 1;
		this is the node child count.

	This is a private, unsafe data structure;
	the correct index and/or child count for the node must always be provided.
	This is not checked.
	*/
	class Node :
	boost::noncopyable // Has special copy constructor and assign method that additionally take Index.
	{	
		// Advances the index to the child node index.
		// Returns the child count of this node, or 0 if the index could not be advanced sufficiently.
		/*
		Cases:
		-	Node index is multi-child (e.g. X0)
			-	Get child count.
			-	Get child step and increment.
		-	Node Index is single-child (e.g. X0v)
			-	Increment (e.g. to X0v0).
			-	Get child count.
			-	Get child step and increment.
		*/
		static char advance(
			Index const & subtreeIndex, char & stepCount, // The index up to step count is that of this node.
			Step & childStep) // Gives the offset of the child.
		{
			char const maximumStepCount = subtreeIndex.getStepCount().offset;
			assert(stepCount <= maximumStepCount && "The step count is out of range.");

			// If we cannot advance, return 0.
			if (maximumStepCount <= stepCount) { return 0; }

			// Get the child count of this node index.
			char childCount = subtreeIndex.getChildCount(Level(stepCount));
			if (1 == childCount) // e.g. X0v
			{
				// If the child count is only 1, stepping to the next child 
				// will result in the "unconsolidated" child and we must step down again.
				// Based on the indexing scheme, a single step is sufficient to find a multi-child.
				// If we cannot advance, return 0.
				assert(0 == subtreeIndex.getSteps()[stepCount]); // e.g. next after X0v is 0.
				++stepCount; // e.g. X0v0
				
				// We're going to need to advance again; bail if we can't.
				if (maximumStepCount <= stepCount) { return 0; }

				// Get the new child count.
				childCount = subtreeIndex.getChildCount(Level(stepCount));
			}
			assert(1 < childCount);

			// Get the child offset for this node and advance.
			childStep = subtreeIndex.getSteps()[stepCount]; // e.g. after X0 is X0v; after X0v0 is X0v0w.
			++stepCount;

			// Return the original child count.
			return childCount;
		}

		// For each value in the 'valuesToVisit' argument:
		// if it is not in results, insert it and call callback.
		static bool visit(
			Set< Value > & results, 
			FunctorInterface< bool, Value > & callback,
			Set< Value > const & valuesToVisit)
		{
			// For each value mapped to this subtree, add it to the result set.
			for (typename Set< Value >::Elements elements(valuesToVisit);
				!elements.getIsEmpty(); elements.popFront())
			{
				Value value = elements.getFront();
				if (!results.find(value))
				{
					results.insert(value);
					if (!callback(value))
					{
						return false;
					}
				}
			}
			return true;
		}

		// The set of values mapped to this subtree.
		Set< Value > values;

		// The set of values mapped to a descendant subtree.
		// If the code is correct, none are in values.
		Set< Value > descendantValues;

		// A node is either a leaf, with no children,
		// or a branch with more than 1 child.
		boost::scoped_array< Node > children;

		// Gets the child count of the node.
		// If child count is 1, steps index to centroid child, sets child count, and sets "advanced" to true.
		// Otherwise, sets "advanced" to false.
		char getChildCount(
			Index & subtreeIndex, // The index of this node.
			bool & advanced) const // Whether or not the index was advanced.
		{
			char childCount = subtreeIndex.getChildCount();
			assert(0 < childCount);
			if (1 == childCount)
			{
				subtreeIndex.stepToChild();
				childCount = subtreeIndex.getChildCount();
				assert(1 < childCount);
				advanced = true;
			} else
			{
				advanced = false;
			}
			return childCount;
		}

		// Removes the child nodes.
		void removeChildren()
		{
			descendantValues.setIsEmpty();
			children.reset();
		}

		// Removes the value from the child subtrees.
		// Removes all children if they are now empty.
		// Expects that there are children.
		// Doesn't check descendantValues to see if this is a redundant call.
		void removeFromChildren(Value value,
			Index & subtreeIndex) // The index of this node.
		{
			assert(children);

			// Get the child count, advancing as necessary.
			bool advanced;
			char childCount = getChildCount(subtreeIndex, advanced);
			assert(1 < childCount);

			// The first (arbitrary) child we do is used as the basis of 
			// comparison for determining determining whether children can be removed.
			subtreeIndex.stepToChild(--childCount);
			Node & child = children[childCount];
			child.remove(value, subtreeIndex);
			assert(child.children || child.descendantValues.getIsEmpty());
			bool canRemoveDescendantValue = !child.descendantValues.find(value);
			bool canRemoveChildren = !child.children;
			subtreeIndex.stepToParent();

			// For each remaining child,
			// remove the value from the subtree and update canRemoveChildren.
			do
			{
				subtreeIndex.stepToChild(--childCount);
				Node & otherChild = children[childCount];
				otherChild.remove(value, subtreeIndex);
				canRemoveDescendantValue = (canRemoveDescendantValue &&
					!otherChild.descendantValues.find(value));
				canRemoveChildren = (canRemoveChildren &&
					!otherChild.children && child.values == otherChild.values);
				subtreeIndex.stepToParent();
			} while (childCount);

			// If we can remove value from descendant values, do so.
			if (canRemoveDescendantValue)
			{
				descendantValues.remove(value);
			}
			
			// If we can remove all children, do so.
			if (canRemoveChildren)
			{
				// Copy the value set that all children share into this node.
				values.insert(child.values);

				// Remove the children.
				removeChildren();
			}
			
			// If we advanced, roll back up.
			if (advanced) { subtreeIndex.stepToParent(); }
		}

		// Inserts the value into each child node.
		void insertInChildren(Value value,
			char childCount) // The child count of this node.
		{
			assert(children);
			assert(1 < childCount);

			descendantValues.insert(value);
			
			do
			{
				children[--childCount].values.insert(value);
			} while (childCount);
		}

		// Visit all the child subtrees, including partials.
		bool visitChildren(
			Set< Value > & results, 
			FunctorInterface< bool, Value > & callback,
			Index & subtreeIndex) const // The index of this node.
		{
			assert(children);

			bool completed = true;

			// Get the child count, advancing as necessary.
			bool advanced;
			char childCount = getChildCount(subtreeIndex, advanced);
			assert(1 < childCount);

			do
			{
				subtreeIndex.stepToChild(--childCount);
				completed = children[childCount].visit(results, callback, &subtreeIndex);
				subtreeIndex.stepToParent();
			} while (completed && childCount);

			// If we advanced, roll back up.
			if (advanced) { subtreeIndex.stepToParent(); }

			return completed;
		}
		
		// Finds the value in child subtrees or a descendants thereof.
		bool findInChildren(Value value,
			Index & subtreeIndex) const // The index of this node.
		{
			assert(children);
			
			bool found = false;
			
			// Get the child count, advancing as necessary.
			bool advanced;
			char childCount = getChildCount(subtreeIndex, advanced);
			assert(1 < childCount);

			do
			{
				found = children[childCount].find(value);
			} while (!found && childCount);

			// If we advanced, roll back up.
			if (advanced) { subtreeIndex.stepToParent(); }

			return found;
		}

		// Finds the value in child subtrees or a descendants thereof,
		// and adds each subtree index to the results.
		SubtreeSet & findInChildren(Value value, SubtreeSet & results,
			Index & subtreeIndex) const // The index of this node.
		{
			assert(children);

			// Get the child count, advancing as necessary.
			bool advanced;
			char childCount = getChildCount(subtreeIndex, advanced);
			assert(1 < childCount);

			do
			{
				subtreeIndex.stepToChild(--childCount);
				children[childCount].find(value, results, subtreeIndex);
				subtreeIndex.stepToParent();
			} while (childCount);

			// If we advanced, roll back up.
			if (advanced) { subtreeIndex.stepToParent(); }

			return results;
		}

	public:

		// Constructs an empty node.
		Node() :
		values(),
		descendantValues(),
		children()
		{}
		
		// Copies the node.
		Node(Node const & node,
			Index & subtreeIndex) :  // The index of this node.
		values(node.values),
		descendantValues(node.descendantValues),
		children()
		{
			if (node.children)
			{
				// Get the child count.
				char childCount = subtreeIndex.getChildCount();
				assert(0 < childCount);

				// Construct the children array.
				children.reset(new Node[childCount]);
				
				// Copy the children.
				// Loop unroll to avoid extra conditional check.
				subtreeIndex.stepToChild(--childCount);
				children[childCount].assign(node.children[childCount], subtreeIndex);
				subtreeIndex.stepToParent();
				while (childCount)
				{
					subtreeIndex.stepToChild(--childCount);
					children[childCount].assign(node.children[childCount], subtreeIndex);
					subtreeIndex.stepToParent();
				}
			}
		}

		// Writes a user-friendly string describing the node structure; for diagnostics.
		void write(std::ostream & output,
			Index & subtreeIndex) const // The index of this node.
		{
			// Write values.
			output << values << ":" << descendantValues;

			// Write children.
			if (children)
			{
				// Calculate indent for each child.
				std::string indent(subtreeIndex.getStepCount().offset, ' ');
				
				// Get child count.
				// If it is 1, insert the centroid, increase the indent, and get new child count.
				bool advanced = false;
				char childCount = subtreeIndex.getChildCount();
				assert(0 < childCount);
				if (1 == childCount)
				{
					subtreeIndex.stepToChild();
					childCount = subtreeIndex.getChildCount();
					
					output << std::endl;
					output << indent << "[0]";
					indent.push_back(' ');
					
					advanced = true;
				}
				assert(1 < childCount);

				// Write the children.
				int childStep = 0;
				do
				{
					subtreeIndex.stepToChild(childStep);
					output << std::endl;
					output << indent << "[" << childStep << "]";
					children[childStep].write(output, subtreeIndex);
					subtreeIndex.stepToParent();
				} while (++childStep < childCount);
				
				// If we advanced, roll back up.
				if (advanced)
				{
					subtreeIndex.stepToParent();
				}
			}
		}

		// Swap the contents of the node.
		void swap(Node & node)
		{
			values.swap(node.values);
			descendantValues.swap(node.descendantValues);
			children.swap(node.children);
		}

		// Assign the node.
		Node & assign(Node const & node,
			Index & subtreeIndex) // The index of this node.
		{
			assert(0 < subtreeIndex.getChildCount() && 
				"Every index must have at least 1 child.");

			Node(node, subtreeIndex).swap(*this);
			return *this;
		}
		
		// Returns true if empty.	
		bool getIsEmpty() const
		{
			assert(children || descendantValues.getIsEmpty());

			return values.getIsEmpty() && !children;
		}

		// Empties the collection.
		void setIsEmpty()
		{
			values.setIsEmpty();
			removeChildren();
		}

	public: // This node

		// Remove the value from this subtree and all descendants.
		// Returns true if the values of this node were modified.
		bool remove(Value value,
			Index & subtreeIndex) // The index of this node.
		{
			// If value is mapped to the subtree...
			if (values.find(value))
			{
				assert(!descendantValues.find(value) &&
					"A value mapped to the subtree should not be explicitly mapped to any descendant.");

				// Remove value mapping and return true due to change.
				values.remove(value);
				return true;
			}

			assert((children || descendantValues.getIsEmpty()) &&
				"A node with no children should not have any descendant subtree values.");
			if (children)
			{
				// If no descendants have the value, there is nothing more to do.
				if (!descendantValues.find(value)) return false;
				descendantValues.remove(value);

				// Remove value from subtrees.
				removeFromChildren(value, subtreeIndex);
			}
			
			// Return false, as this node didn't change.
			return false;
		}
		
		// For each value mapped to this subtree:
		// if it is not in results, insert it and call callback.
		// If subtreeIndex is provided, visits all partials.
		bool visit(
			Set< Value > & results, 
			FunctorInterface< bool, Value > & callback,
			Index * subtreeIndex = 0) const // The index of this node.
		{
			if (!visit(results, callback, values)) { return false; }
			if (!subtreeIndex || !children) { return true; }
			return visit(results, callback, descendantValues);
		}

		// Returns true if the value is mapped to this subtree
		// or one of its descendants.
		bool find(Value value) const // The index of this node.
		{
			if (values.find(value)) { return true; }
			if (!children) { return false; }
			return descendantValues.find(value);
		}

		// Finds the subtrees explicitly mapped to this value.
		// The 'subtreeIndex' argument is the index of this node.
		// Returns a reference to the results.
		SubtreeSet & find(Value value, SubtreeSet & results,
			Index & subtreeIndex) const // The index of this node.
		{
			if (values.find(value)) { results.insert(subtreeIndex); }
			if (!children) { return results; }
			if (!descendantValues.find(value)) { return results; }
			return findInChildren(value, results, subtreeIndex);
		}

	public: // Non-strict descendant node

		// Inserts the mapping of the value to the given descendant subtree.
		// Returns true if any node values have changed.
		bool insert(
			Index const & subtreeIndex, char stepCount, // The index up to step count is that of this node.
			Value value)
		{
			assert(stepCount <= subtreeIndex.getStepCount().offset &&
				"The step count is out of range.");

			// If it is already mapped to the entire subtree, nothing to do.
			if (values.find(value)) { return false; }
			
			Step childStep;
			if (char childCount = advance(subtreeIndex, stepCount, childStep))
			{
				assert(stepCount);
				assert(stepCount <= subtreeIndex.getStepCount().offset);
				assert(1 < childCount);
				assert(childStep < childCount);

				// One way or another, it is going to be inserted as a descendant.
				descendantValues.insert(value);

				// If there are no children, create children, defer to child. and return false.
				// Value consolidation is not possible in this case, so no need to check.
				if (!children)
				{
					children.reset(new Node[childCount]);
					children[childStep].insert(subtreeIndex, stepCount, value);
					return false;
				}

				// Defer to child.  If there was no change to the node values, return false.
				assert(children);
				Node & child = children[childStep];
				if (!child.insert(subtreeIndex, stepCount, value))
				{
					return false;
				}

				// Determine whether value/children can be consolidated.
				// If the value is not mapped to every child subtree, no consolidation is possible.
				assert(child.values.find(value));
				bool canRemoveChildren = !child.children;
				for (Step childOffset = childCount; childStep < --childOffset; )
				{
					Node & otherChild = children[childOffset];
					if (!otherChild.values.find(value)) { return false; }
					canRemoveChildren = (canRemoveChildren &&
						!otherChild.children && child.values == otherChild.values);
				}
				while (childStep)
				{
					Node & otherChild = children[--childStep];
					if (!otherChild.values.find(value)) { return false; }
					canRemoveChildren = (canRemoveChildren &&
						!otherChild.children && child.values == otherChild.values);
				}
				
				// Consolidate.
				values.insert(value);
				if (canRemoveChildren)
				{
					// Copy the value set that all children share into this node,
					// and remove the children.
					values.insert(child.values);
					removeChildren();
					assert(descendantValues.getIsEmpty());
				} else
				{
					// Remove the value from descendants.
					descendantValues.remove(value);

					// Remove the value from each child.
					assert(1 < childCount);
					do
					{
						children[--childCount].values.remove(value);
					} while (childCount);
				}

				// The values have changed for this node.
				return true;
			}

			// Map this whole subtree to value;
			// remove from descendant subtrees.
			values.insert(value);
			descendantValues.remove(value);

			if (children)
			{
				// Remove from child subtrees.  Removes children if they become empty.
				Index index(subtreeIndex);
				removeFromChildren(value, index);
			}

			// The values have changed for this node.
			return true;
		}

		// Removes the mapping of the value from the given descendant subtree.
		// Returns true if any node values have changed.
		bool remove(
			Index const & subtreeIndex, char stepCount, // The index up to step count is that of this node.
			Value value)
		{
			assert(stepCount <= subtreeIndex.getStepCount().offset &&
				"The step count is out of range.");

			// If the value does not exist in this tree, nothing more to do.
			if (!values.find(value) && !descendantValues.find(value))
			{
				return false;
			}

			Step childStep;	
			if (char const childCount = advance(subtreeIndex, stepCount, childStep))
			{
				assert(stepCount);
				assert(stepCount <= subtreeIndex.getStepCount().offset);
				assert(1 < childCount);
				assert(childStep < childCount);

				// If the value is mapped to the entire subtree:
				if (values.find(value))
				{
					// Remove value from this node.
					values.remove(value);
					
					// If there are no children, create them so that we 
					// can map the value to the siblings.
					if (!children)
					{
						children.reset(new Node[childCount]);
					}

					// Insert in all child nodes.
					insertInChildren(value, childCount);

					// Remove from child.
					// There are no consolidation opportunities here.
					assert(children);
					children[childStep].remove(subtreeIndex, stepCount, value);

					// Return true, since the values have changed.
					return true;
				}

				// The value is not mapped to the entire subtree.
				// If there are children:
				assert(!values.find(value));
				if (children)
				{
					// Remove from child.
					// Call remove on child, which may create an 
					// opportunity for value consolidation.
					Node & child = children[childStep];
					if (!child.remove(subtreeIndex, stepCount, value))
					{
						// No change in child values; return false.
						return false;
					}
					assert(!child.values.find(value));

					// Determine whether value/children can be consolidated.
					// If the value is mapped to any child subtree, no consolidation is possible.
					assert(!child.values.find(value));
					assert(child.children || child.descendantValues.getIsEmpty());
					bool canRemoveDescendantValue = !child.descendantValues.find(value);
					bool canRemoveChildren = !child.children;
					for (Step childOffset = childCount; childStep < --childOffset; )
					{
						Node & otherChild = children[childOffset];
						if (otherChild.values.find(value)) { return false; }
						canRemoveDescendantValue = (canRemoveDescendantValue &&
							!otherChild.descendantValues.find(value));
						canRemoveChildren = (canRemoveChildren &&
							!otherChild.children && child.values == otherChild.values);
					}
					while (childStep)
					{
						Node & otherChild = children[--childStep];
						if (otherChild.values.find(value)) { return false; }
						canRemoveDescendantValue = (canRemoveDescendantValue &&
							!otherChild.descendantValues.find(value));
						canRemoveChildren = (canRemoveChildren &&
							!otherChild.children && child.values == otherChild.values);
					}
					
					// Consolidate.
					if (canRemoveChildren)
					{
						// Copy the value set that all children share into this node.
						values.insert(child.values);
						
						// Remove the children.
						removeChildren();
					} else if (canRemoveDescendantValue)
					{
						// Remove the value from descendants.
						descendantValues.remove(value);
					}

					// The values have changed for this node.
					return true;
				}

				// It wasn't mapped to this subtree, and there are no children.
				assert(!values.find(value));
				assert(!children);
				assert(descendantValues.getIsEmpty());
				return false;
			}

			// Remove the value from this node and all descendant subtrees.
			Index index(subtreeIndex);
			return remove(value, index);
		}

		// For each value mapped to the given descendant subtree,
		// or a descendant thereof:
		// if it is not in results, insert it and call callback.
		bool visit(
			Index const & subtreeIndex, char stepCount, // The index up to step count is that of this node.
			Set< Value > & results, 
			FunctorInterface< bool, Value > & callback,
			bool includePartial) const
		{
			assert(stepCount <= subtreeIndex.getStepCount().offset &&
				"The step count is out of range.");

			// Visit values mapped to the whole subtree.
			if (!visit(results, callback)) return false;
			
			// Visit child if necessary.
			if (children)
			{
				// If we can advance the index, continue to the child.
				Step childStep;
				if (char const childCount = advance(subtreeIndex, stepCount, childStep))
				{
					// Visit values mapped within the child subtree.
					assert(1 < childCount);
					assert(childStep < childCount);
					return children[childStep].visit(
						subtreeIndex, stepCount, results, callback, includePartial);
				}
				
				if (includePartial)
				{
					// Visit the values mapped to descendants.
					return visit(results, callback, descendantValues);
				}
			}
			return true;
		}

		// Populates the results with all values mapped to the given descendant subtree
		// (and its descendants, if specified),
		// and returns a reference to the results.
		Set< Value > & find(
			Index const & subtreeIndex, char stepCount, // The index up to step count is that of this node.
			Set< Value > & results,
			bool includePartial) const
		{
			struct : FunctorInterface< bool, Value >
			{
				bool operator ()(Value value) { return true; }
			} callback;
			visit(subtreeIndex, stepCount, results, callback, includePartial);
			return results;
		}

		// Returns true if the value is mapped to the given descendant subtree
		// (or a descendant thereof, if specified).
		bool find(
			Index const & subtreeIndex, char stepCount, // The index up to step count is that of this node.
			Value value,
			bool includePartial) const
		{
			// Use the visitor; stops (and returns false) if found.
			Set< Value > results;
			struct Callback : FunctorInterface< bool, Value >
			{
				Value value;
				explicit Callback(Value value) : value(value) {}
				bool operator ()(Value visitee) { return (visitee != value); }
			} callback(value);
			return !visit(subtreeIndex, stepCount, results, callback, includePartial);
		}

		// Returns true if there is at least one value mapped to the given descendant subtree
		// (or a descendant thereof, if specified).
		bool find(
			Index const & subtreeIndex, char stepCount,
			bool includePartial) const // The index up to step count is that of this node.
		{
			// Use the visitor; stops (and returns false) if found.
			Set< Value > results;
			struct : FunctorInterface< bool, Value >
			{
				bool operator ()(Value visitee) { return false; }
			} callback;
			return !visit(subtreeIndex, stepCount, results, callback, includePartial);
		}

	} node;	

#endif

public:

	class Test;

	// Constructs an empty multimap.
	SubtreeMultimap() : node() {}
	
	// Copies the multimap.
	SubtreeMultimap(SubtreeMultimap const & that) : node()
	{
		Index subtreeIndex;
		Node(that.node, subtreeIndex).swap(node);
	}

	friend std::ostream & operator <<(std::ostream & output, SubtreeMultimap const & multimap)
	{
		output << "(";
		{
			Index subtreeIndex;
			multimap.node.write(output, subtreeIndex);
		}
		output << ")";
		return output;
	}

	void swap(SubtreeMultimap & with)
	{
		node.swap(with.node);
	}
	
	SubtreeMultimap & operator =(SubtreeMultimap that)
	{
		swap(that);
		return *this;
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

	size_t getCount() const
	{
		assert(0); throw std::exception(); // TODO
	}

	// Inserts the given value mapping for the given subtree.
	// If all of the siblings are mapped to the value,
	// move the value mapping to the parent set
	// and if the parent's children are now leaves with empty
	// sets, remove them.
	void insert(Index const & subtreeIndex, Value value)
	{
		node.insert(subtreeIndex, 0, value);
	}

	// Removes the given value mapping from the given subtree.
	// If the value is mapped to an ancestor, map the value to 
	// all siblings, creating as necessary.
	void remove(Index const & subtreeIndex, Value value)
	{
		node.remove(subtreeIndex, 0, value);
	}

	// Removes the value.
	void remove(Value value)
	{
		Index subtreeIndex;
		node.remove(value, subtreeIndex);
	}

	// Finds the subtree and populates the value set.
	MutableSetInterface< Value > & find(Index const & subtreeIndex, 
		MutableSetInterface< Value > & results,
		bool includePartial) const
	{
		return node.find(subtreeIndex, 0, results, includePartial);
	}

	bool find(Index const & subtreeIndex,
		bool includePartial) const
	{
		return node.find(subtreeIndex, 0, includePartial);
	}
	
	// Returns true if the value is found at the index.
	bool find(Index const & subtreeIndex, Value value,
		bool includePartial) const
	{
		return node.find(subtreeIndex, 0, value, includePartial);
	}

	// Finds the value and populates the subtree set.
	SubtreeSet & find(Value value, SubtreeSet & results) const
	{
		Index subtreeIndex;
		return node.find(value, results, subtreeIndex);
	}

	// Returns true if the value is found in the subtree.
	bool find(Value value) const
	{
		return node.find(value);
	}

	// Visits the values in the given subtree.
	// If includePartial is true,
	// visits values in any descendant subtree as well.
	bool visit(Index const & subtreeIndex, 
		MutableSetInterface< Value > & results,
		FunctorInterface< bool, Value > & callback,
		bool includePartial) const
	{
		return node.visit(subtreeIndex, 0, results, callback, includePartial);
	}
};

#else //////////////////////////////////////////////////////////////

// Space-optimized is impractically slow so far.
//#define SUBTREE_MULTIMAP_SPACE_OPTIMIZED

// TODO: Make this a specialization of Multimap< Subtree const &, Value >,
// once Subtree and Multimap are implemented.
template < typename Value >
class Pyxis::Globe::Tree::SubtreeMultimap : public virtual Pointee
{
	/*
	A node represents a subtree.

	This is a private, unsafe data structure; the correct
	index and/or child count must always be provided
	because it is not stored.

	Storage is optimized such that steps with one child are not stored.

	If a value is stored in the node, it indicates one of the following:
	-	If at least one child node also has the value,
		this indicates a partial subtree for the value.
	-	If no child nodes have the value,
		this indicates a full subtree for the value.
	*/
	class Node : boost::noncopyable
	{
		// Steps the index to the child and continues stepping
		// while there is only 1 child.
		static void stepToChild(Index & subtreeIndex, Step const childStep = 0)
		{
			assert(1 < subtreeIndex.getChildCount());

			// Step to the child.
			subtreeIndex.stepToChild(childStep);
			assert(subtreeIndex.getChildCount());

			// If it has only 1 child, step to it.
			// Note that we cannot just do an "is vertex" check here,
			// because index "s00" has only one child.
			if (1 == subtreeIndex.getChildCount())
			{
				subtreeIndex.stepToChild();
				assert(1 < subtreeIndex.getChildCount());
			}
		}
		
		// Steps the index to the parent and continues stepping
		// while the parent only has 1 child.
		static void stepToParent(Index & subtreeIndex)
		{
			assert(1 < subtreeIndex.getChildCount());

			// Step to the parent.
			subtreeIndex.stepToParent();
			assert(subtreeIndex.getChildCount());
			
			// If it has only 1 child, step to its parent.
			// Note that we cannot just do an "is vertex" check here,
			// because index "s00" has only one child.
			if (1 == subtreeIndex.getChildCount())
			{
				subtreeIndex.stepToParent();
				assert(1 < subtreeIndex.getChildCount());
			}
		}

		// Advances the index to the next prefix that has a child count
		// greater than 1 and return true, or return false if we run out 
		// of index.
		static bool advance(
			Index const & subtreeIndex, char & stepCount, // The index up to step count is that of this node.
			Step & childStep)
		{
			assert(1 < subtreeIndex.getChildCount(Level(stepCount)));
			assert(1 < subtreeIndex.getChildCount());

			char const maximumStepCount = subtreeIndex.getStepCount().offset;
			assert(stepCount <= maximumStepCount &&
				"The step count is out of range.");

			// If we can advance...
			if (stepCount < maximumStepCount)
			{
				// Advance and get the child count to determine if we need to 
				// keep descending.
				// Note that we cannot just do an "is vertex" check here,
				// because index "s00" has only one child.
				char const childCount = subtreeIndex.getChildCount(Level(++stepCount));
				assert(childCount);

				// Get the child step at this position in the index.
				childStep = subtreeIndex.getSteps()[stepCount - 1];

				// If there is one child, advance again.  If not possible, return false.
				if (1 == childCount)
				{
					assert(stepCount < maximumStepCount &&
						"The index passed in is supposed to have more than 1 child.");
					++stepCount;
				}

				assert(1 < subtreeIndex.getChildCount(Level(stepCount)));
				return true;
			}

			return false;
		}

		// For each value in the 'valuesToVisit' argument:
		// if it is not in results, insert it and call callback.
		static bool visit(
			Set< Value > & results, 
			FunctorInterface< bool, Value > & callback,
			Set< Value > const & valuesToVisit)
		{
			// For each value mapped to this subtree, add it to the result set.
			for (typename Set< Value >::Elements elements(valuesToVisit);
				!elements.getIsEmpty(); elements.popFront())
			{
				Value value = elements.getFront();
				if (!results.find(value))
				{
					results.insert(value);
					if (!callback(value))
					{
						return false;
					}
				}
			}
			return true;
		}

		// The set of values mapped to this subtree.
		Set< Value > values;
		
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
		// The set of values mapped to a descendant subtree.
		// If the code is correct, none are in values.
		Set< Value > descendantValues;
#endif
		
		// A node is either a leaf, with no children,
		// or a branch with more than 1 child.
		boost::scoped_array< Node > children;

		// Removes the child nodes.
		void removeChildren()
		{
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
			descendantValues.setIsEmpty();
#endif
			children.reset();
		}

		// Removes the value from the child subtrees.
		// Removes all children if they are now empty.
		// Expects that there are children.
		// Doesn't check descendantValues to see if this is a redundant call.
		void removeFromChildren(Value value,
			Index & subtreeIndex) // The index of this node.
		{
			assert(children);

			// Get the child count, and assert that it is greater than 1.
			char childCount = subtreeIndex.getChildCount();
			assert(1 < childCount);

			// Loop unroll to avoid extra conditional check.
			// Use this child as the basis of comparison for determining
			// whether children can be removed.
			stepToChild(subtreeIndex, --childCount);
			Node & child = children[childCount];
			child.remove(value, subtreeIndex);
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
			assert(child.children || child.descendantValues.getIsEmpty());
			bool canRemoveDescendantValue = !child.descendantValues.find(value);
#endif
			bool canRemoveChildren = !child.children;
			stepToParent(subtreeIndex);

			// For each remaining child,
			// remove the value from the subtree and update canRemoveChildren.
			do
			{
				stepToChild(subtreeIndex, --childCount);
				Node & otherChild = children[childCount];
				otherChild.remove(value, subtreeIndex);
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
				canRemoveDescendantValue = (canRemoveDescendantValue &&
					!otherChild.descendantValues.find(value));
#endif
				canRemoveChildren = (canRemoveChildren &&
					!otherChild.children && child.values == otherChild.values);
				stepToParent(subtreeIndex);
			} while (childCount);

#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
			if (canRemoveDescendantValue)
			{
				descendantValues.remove(value);
			}
#endif
			if (canRemoveChildren)
			{
				// Copy the value set that all children share into this node.
				values.insert(child.values);

				// Remove the children.
				removeChildren();
			}
		}

		// Inserts the value into each child nodes.
		void insertInChildren(Value value,
			char childCount) // The child count of this node.
		{
			assert(children);
			assert(1 < childCount);

#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
			descendantValues.insert(value);
#endif
			
			// Loop unroll to avoid extra conditional check.
			children[--childCount].values.insert(value);
			do
			{
				children[--childCount].values.insert(value);
			} while (childCount);
		}

		// Visit all the child subtrees, including partials.
		bool visitChildren(
			Set< Value > & results, 
			FunctorInterface< bool, Value > & callback,
			Index & subtreeIndex) const // The index of this node.
		{
			assert(children);
			
			// Get the child count, and assert that it is greater than 1.
			char childCount = subtreeIndex.getChildCount();
			assert(1 < childCount);

			while (childCount)
			{
				stepToChild(subtreeIndex, --childCount);
				bool const completed = children[childCount].visit(results, callback, &subtreeIndex);
				stepToParent(subtreeIndex);
				if (!completed) { return false; }
			}
			return true;
		}
		
		// Finds the value in child subtrees or a descendants thereof.
		bool findInChildren(Value value,
			Index & subtreeIndex) const // The index of this node.
		{
			assert(children);
			
			// Get the child count, and assert that it is greater than 1.
			char childCount = subtreeIndex.getChildCount();
			assert(1 < childCount);

			while (childCount)
			{
				stepToChild(subtreeIndex, --childCount);
				bool const found = children[childCount].find(value, &subtreeIndex);
				stepToParent(subtreeIndex);
				if (found) { return true; }
			}
			return false;
		}

		// Finds the value in child subtrees or a descendants thereof,
		// and adds each subtree index to the results.
		SubtreeSet & findInChildren(Value value, SubtreeSet & results,
			Index & subtreeIndex) const // The index of this node.
		{
			assert(children);

			// Get the child count, and assert that it is greater than 1.
			char childCount = subtreeIndex.getChildCount();
			assert(1 < childCount);

			while (childCount)
			{
				stepToChild(subtreeIndex, --childCount);
				children[childCount].find(value, results, subtreeIndex);
				stepToParent(subtreeIndex);
			}
			return results;
		}

	public:

		// Constructs an empty node.
		Node() :
		values(),
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
		descendantValues(),
#endif
		children()
		{}
		
		// Copies the node.
		Node(Node const & node,
			Index & subtreeIndex) :  // The index of this node.
		values(node.values),
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
		descendantValues(node.descendantValues),
#endif
		children()
		{
			if (node.children)
			{
				// Get the child count.
				char childCount = subtreeIndex.getChildCount();
				assert(1 < childCount);
	
				// Construct the children array.
				children.reset(new Node[childCount]);
				
				// Copy the children.
				// Loop unroll to avoid extra conditional check.
				stepToChild(subtreeIndex, --childCount);
				children[childCount].assign(node.children[childCount], subtreeIndex);
				stepToParent(subtreeIndex);
				do
				{
					stepToChild(subtreeIndex, --childCount);
					children[childCount].assign(node.children[childCount], subtreeIndex);
					stepToParent(subtreeIndex);
				} while (childCount);
			}
		}

		// Writes a user-friendly string describing the node structure; for diagnostics.
		void write(std::ostream & output,
			Index & subtreeIndex) const // The index of this node.
		{
			output << values;
			
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
			output << ':' << descendantValues;
#endif

			if (children)
			{
				char const childCount = subtreeIndex.getChildCount();
				assert(1 < childCount);
				for (int childStep = 0; childStep < childCount; ++childStep)
				{
					subtreeIndex.stepToChild(childStep);
					output << std::endl;
					output << std::string(subtreeIndex.getStepCount().offset, ' ') << "[" << childStep << "]";

					if (1 == subtreeIndex.getChildCount())
					{
						subtreeIndex.stepToChild();
						output << std::endl;
						output << std::string(subtreeIndex.getStepCount().offset, ' ') << "[0]";

						assert(1 < subtreeIndex.getChildCount());
						children[childStep].write(output, subtreeIndex);
						subtreeIndex.stepToParent();
					} else
					{
						children[childStep].write(output, subtreeIndex);
					}

					subtreeIndex.stepToParent();
				}
			}
		}
		
		// Swap the contents of the node.
		void swap(Node & node)
		{
			values.swap(node.values);
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
			descendantValues.swap(node.descendantValues);
#endif
			children.swap(node.children);
		}

		// Assign the node.
		Node & assign(Node const & node,
			Index & subtreeIndex) // The index of this node.
		{
			assert(1 < subtreeIndex.getChildCount() && 
				"The caller is supposed to guarantee that the index has more than 1 child.");

			Node(node, subtreeIndex).swap(*this);
			return *this;
		}
		
		// Returns true if empty.	
		bool getIsEmpty() const
		{
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
			assert(children || descendantValues.getIsEmpty());
#endif
			return values.getIsEmpty() && !children;
		}

		// Empties the collection.
		void setIsEmpty()
		{
			values.setIsEmpty();
			removeChildren();
		}

	public: // This node

		// Remove the value from this subtree and all descendants.
		// Returns true if the values of this node were modified.
		bool remove(Value value,
			Index & subtreeIndex) // The index of this node.
		{
			assert(1 < subtreeIndex.getChildCount() && 
				"The caller is supposed to guarantee that the index has more than 1 child.");

			// If value is mapped to the subtree...
			if (values.find(value))
			{
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
				assert(!descendantValues.find(value) &&
					"A value mapped to the subtree should not be explicitly mapped to any descendant.");
#endif
				// Remove value mapping and return true due to change.
				values.remove(value);
				return true;
			}

#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
			assert((children || descendantValues.getIsEmpty()) &&
				"A node with no children should not have any descendant subtree values.");
#endif
			if (children)
			{
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
				// If no descendants have the value, there is nothing more to do.
				if (!descendantValues.find(value)) return false;
				descendantValues.remove(value);
#endif
				// Remove value from subtrees.
				removeFromChildren(value, subtreeIndex);
			}
			
			// Return false, as this node didn't change.
			return false;
		}
		
		// For each value mapped to this subtree:
		// if it is not in results, insert it and call callback.
		// If subtreeIndex is provided, visits all partials.
		bool visit(
			Set< Value > & results, 
			FunctorInterface< bool, Value > & callback,
			Index * subtreeIndex = 0) const // The index of this node.
		{
			if (!visit(results, callback, values)) { return false; }
			if (!subtreeIndex || !children) { return true; }
#if defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
			return visitChildren(results, callback, *subtreeIndex);
#else
			return visit(results, callback, descendantValues);
#endif
		}

		// Returns true if the value is mapped to this subtree
		// or one of its descendants.
		bool find(Value value,
			Index & subtreeIndex) const // The index of this node.
		{
			if (values.find(value)) { return true; }
			if (!children) { return false; }
#if defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
			return findInChildren(value, *subtreeIndex);
#else
			return descendantValues.find(value);
#endif
		}

		// Finds the subtrees explicitly mapped to this value.
		// The 'subtreeIndex' argument is the index of this node.
		// Returns a reference to the results.
		SubtreeSet & find(Value value, SubtreeSet & results,
			Index & subtreeIndex) const // The index of this node.
		{
			if (values.find(value)) { results.insert(subtreeIndex); }
			if (!children) { return results; }
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
			if (!descendantValues.find(value)) { return results; }
#endif
			return findInChildren(value, results, subtreeIndex);
		}

	public: // Non-strict descendant node

		// Inserts the mapping of the value to the given descendant subtree.
		// Returns true if any node values have changed.
		bool insert(
			Index const & subtreeIndex, char stepCount, // The index up to step count is that of this node.
			Value value)
		{
			assert(1 < subtreeIndex.getChildCount() && 
				"The caller is supposed to guarantee that the index has more than 1 child.");
			assert(stepCount <= subtreeIndex.getStepCount().offset &&
				"The step count is out of range.");

			// If it is already mapped to the entire subtree, nothing to do.
			if (values.find(value)) { return false; }

			// Cache the child count before advancing.
			char childCount = subtreeIndex.getChildCount(Level(stepCount));
			assert(1 < childCount &&
				"The child count of the index prefix must be greater than 1.");

			Step childStep;
			if (advance(subtreeIndex, stepCount, childStep))
			{
				assert(stepCount);
				assert(stepCount <= subtreeIndex.getStepCount().offset);
				assert(childStep < childCount);

#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
				// One way or another, it is going to be inserted as a descendant.
				descendantValues.insert(value);
#endif

				// If there are no children, create children, defer to child. and return false.
				// Value consolidation is not possible in this case, so no need to check.
				if (!children)
				{
					children.reset(new Node[childCount]);
					children[childStep].insert(subtreeIndex, stepCount, value);
					return false;
				}

				// Defer to child.  If there was no change to the node values, return false.
				assert(children);
				Node & child = children[childStep];
				if (!child.insert(subtreeIndex, stepCount, value))
				{
					return false;
				}

				// Determine whether value/children can be consolidated.
				// If the value is not mapped to every child subtree, no consolidation is possible.
				assert(child.values.find(value));
				bool canRemoveChildren = !child.children;
				for (Step childOffset = childCount; childStep < --childOffset; )
				{
					Node & otherChild = children[childOffset];
					if (!otherChild.values.find(value)) { return false; }
					canRemoveChildren = (canRemoveChildren &&
						!otherChild.children && child.values == otherChild.values);
				}
				while (childStep)
				{
					Node & otherChild = children[--childStep];
					if (!otherChild.values.find(value)) { return false; }
					canRemoveChildren = (canRemoveChildren &&
						!otherChild.children && child.values == otherChild.values);
				}
				
				// Consolidate.
				values.insert(value);
				if (canRemoveChildren)
				{
					// Copy the value set that all children share into this node,
					// and remove the children.
					values.insert(child.values);
					removeChildren();
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
					assert(descendantValues.getIsEmpty());
#endif
				} else
				{
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
					// Remove the value from descendants.
					descendantValues.remove(value);
#endif
					// Remove the value from each child.
					assert(1 < childCount);
					do
					{
						children[--childCount].values.remove(value);
					} while (childCount);
				}

				// The values have changed for this node.
				return true;
			}

			assert(stepCount == subtreeIndex.getStepCount().offset);

			// Map this whole subtree to value;
			// remove from descendant subtrees.
			values.insert(value);
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
			descendantValues.remove(value);
#endif
			if (children)
			{
				// Remove from child subtrees.  Removes children if they become empty.
				Index index(subtreeIndex);
				removeFromChildren(value, index);
			}

			// The values have changed for this node.
			return true;
		}

		// Removes the mapping of the value from the given descendant subtree.
		// Returns true if any node values have changed.
		bool remove(
			Index const & subtreeIndex, char stepCount, // The index up to step count is that of this node.
			Value value)
		{
			assert(1 < subtreeIndex.getChildCount() && 
				"The caller is supposed to guarantee that the index has more than 1 child.");
			assert(stepCount <= subtreeIndex.getStepCount().offset &&
				"The step count is out of range.");

#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
			// If the value does not exist in this tree, nothing more to do.
			if (!values.find(value) && !descendantValues.find(value))
			{
				return false;
			}
#endif

			// Cache the child count before advancing.
			char childCount = subtreeIndex.getChildCount(Level(stepCount));
			assert(1 < childCount &&
				"The child count of the index prefix must be greater than 1.");

			Step childStep;	
			if (advance(subtreeIndex, stepCount, childStep))
			{
				assert(stepCount);
				assert(stepCount <= subtreeIndex.getStepCount().offset);
				assert(childStep < childCount);

				// If the value is mapped to the entire subtree:
				if (values.find(value))
				{
					// Remove value from this node.
					values.remove(value);

					// If there are no children, create them so that we 
					// can map the value to the siblings.
					if (!children)
					{
						children.reset(new Node[childCount]);
					}

					// Insert in all child nodes.
					insertInChildren(value, childCount);

					// Remove from child.
					// There are no consolidation opportunities here.
					assert(children);
					children[childStep].remove(subtreeIndex, stepCount, value);
					
					// Return true, since the values have changed.
					return true;
				}

				// The value is not mapped to the entire subtree.
				// If there are children:
				assert(!values.find(value));
				if (children)
				{
					// Remove from child.
					// Call remove on child, which may create an 
					// opportunity for value consolidation.
					Node & child = children[childStep];
					if (!child.remove(subtreeIndex, stepCount, value))
					{
						// No change in child values; return false.
						return false;
					}
					assert(!child.values.find(value));

					// Determine whether value/children can be consolidated.
					// If the value is mapped to any child subtree, no consolidation is possible.
					assert(!child.values.find(value));
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
					assert(child.children || child.descendantValues.getIsEmpty());
					bool canRemoveDescendantValue = !child.descendantValues.find(value);
#endif
					bool canRemoveChildren = !child.children;
					for (Step childOffset = childCount; childStep < --childOffset; )
					{
						Node & otherChild = children[childOffset];
						if (otherChild.values.find(value)) { return false; }
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
						canRemoveDescendantValue = (canRemoveDescendantValue &&
							!otherChild.descendantValues.find(value));
#endif
						canRemoveChildren = (canRemoveChildren &&
							!otherChild.children && child.values == otherChild.values);
					}
					while (childStep)
					{
						Node & otherChild = children[--childStep];
						if (otherChild.values.find(value)) { return false; }
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
						canRemoveDescendantValue = (canRemoveDescendantValue &&
							!otherChild.descendantValues.find(value));
#endif
						canRemoveChildren = (canRemoveChildren &&
							!otherChild.children && child.values == otherChild.values);
					}
					
					// Consolidate.
					if (canRemoveChildren)
					{
						// Copy the value set that all children share into this node.
						values.insert(child.values);
						
						// Remove the children.
						removeChildren();
					} else
					{
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
						// Remove the value from descendants, if possible.
						if (canRemoveDescendantValue)
						{
							descendantValues.remove(value);
						}
#endif
					}

					// The values have changed for this node.
					return true;
				}

				// It wasn't mapped to this subtree, and there are no children.
				assert(!values.find(value));
				assert(!children);
#if !defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
				assert(descendantValues.getIsEmpty());
#endif
				return false;
			}

			assert(stepCount == subtreeIndex.getStepCount().offset);

			// Remove the value from this node and all descendant subtrees.
			Index index(subtreeIndex);
			return remove(value, index);
		}

		// For each value mapped to the given descendant subtree,
		// or a descendant thereof:
		// if it is not in results, insert it and call callback.
		bool visit(
			Index const & subtreeIndex, char stepCount, // The index up to step count is that of this node.
			Set< Value > & results, 
			FunctorInterface< bool, Value > & callback,
			bool includePartial) const
		{
			assert(1 < subtreeIndex.getChildCount() && 
				"The caller is supposed to guarantee that the index has more than 1 child.");
			assert(stepCount <= subtreeIndex.getStepCount().offset &&
				"The step count is out of range.");

			// Visit values mapped to the whole subtree.
			if (!visit(results, callback)) return false;
			
			// Visit child if necessary.
			if (children)
			{
				// Cache the child count before advancing.
				char const childCount = subtreeIndex.getChildCount(Level(stepCount));
				assert(1 < childCount &&
					"The child count of the index prefix must be greater than 1.");

				// If we can advance the index, continue to the child.
				Step childStep;
				if (advance(subtreeIndex, stepCount, childStep))
				{
					// Visit values mapped within the child subtree.
					assert(childStep < childCount);
					return children[childStep].visit(
						subtreeIndex, stepCount, results, callback, includePartial);
				}
				
				if (includePartial)
				{
#if defined(SUBTREE_MULTIMAP_SPACE_OPTIMIZED)
					// Visit all the child subtrees, down to the leaves.
					Index index(subtreeIndex);
					return visitChildren(results, callback, index);
#else
					// Visit the values mapped to descendants.
					return visit(results, callback, descendantValues);
#endif
				}
			}
			return true;
		}

		// Populates the results with all values mapped to the given descendant subtree
		// (and its descendants, if specified),
		// and returns a reference to the results.
		Set< Value > & find(
			Index const & subtreeIndex, char stepCount, // The index up to step count is that of this node.
			Set< Value > & results,
			bool includePartial) const
		{
			struct : FunctorInterface< bool, Value >
			{
				bool operator ()(Value value) { return true; }
			} callback;
			visit(subtreeIndex, stepCount, results, callback, includePartial);
			return results;
		}

		// Returns true if the value is mapped to the given descendant subtree
		// (or a descendant thereof, if specified).
		bool find(
			Index const & subtreeIndex, char stepCount, // The index up to step count is that of this node.
			Value value,
			bool includePartial) const
		{
			// Use the visitor; stops (and returns false) if found.
			Set< Value > results;
			struct Callback : FunctorInterface< bool, Value >
			{
				Value value;
				explicit Callback(Value value) : value(value) {}
				bool operator ()(Value visitee) { return (visitee != value); }
			} callback(value);
			return !visit(subtreeIndex, stepCount, results, callback, includePartial);
		}

		// Returns true if there is at least one value mapped to the given descendant subtree
		// (or a descendant thereof, if specified).
		bool find(
			Index const & subtreeIndex, char stepCount,
			bool includePartial) const // The index up to step count is that of this node.
		{
			// Use the visitor; stops (and returns false) if found.
			Set< Value > results;
			struct : FunctorInterface< bool, Value >
			{
				bool operator ()(Value visitee) { return false; }
			} callback;
			return !visit(subtreeIndex, stepCount, results, callback, includePartial);
		}

	} node;
	
public:

	class Test;

	// Constructs an empty multimap.
	SubtreeMultimap() : node() {}
	
	// Copies the multimap.
	SubtreeMultimap(SubtreeMultimap const & that) : node()
	{
		Index subtreeIndex;
		assert(1 < subtreeIndex.getChildCount());
		Node(that.node, subtreeIndex).swap(node);
	}

	friend std::ostream & operator <<(std::ostream & output, SubtreeMultimap const & multimap)
	{
		output << "(";
		{
			Index subtreeIndex;
			multimap.node.write(output, subtreeIndex);
		}
		output << ")";
		return output;
	}

	void swap(SubtreeMultimap & with)
	{
		node.swap(with.node);
	}
	
	SubtreeMultimap & operator =(SubtreeMultimap that)
	{
		swap(that);
		return *this;
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

	// Inserts the given value mapping for the given subtree.
	// If all of the siblings are mapped to the value,
	// move the value mapping to the parent set
	// and if the parent's children are now leaves with empty
	// sets, remove them.
	void insert(Index const & subtreeIndex, Value value)
	{
		if (1 < subtreeIndex.getChildCount())
		{
			node.insert(subtreeIndex, 0, value);
		} else
		{
			Index index(subtreeIndex);
			index.stepToChild();
			assert(1 < index.getChildCount());
			node.insert(index, 0, value);
		}
	}

	// Removes the given value mapping from the given subtree.
	// If the value is mapped to an ancestor, map the value to 
	// all siblings, creating as necessary.
	void remove(Index const & subtreeIndex, Value value)
	{
		if (1 < subtreeIndex.getChildCount())
		{
			node.remove(subtreeIndex, 0, value);
		} else
		{
			Index index(subtreeIndex);
			index.stepToChild();
			assert(1 < index.getChildCount());
			node.remove(index, 0, value);
		}
	}

	// Removes the value.
	void remove(Value value)
	{
		Index subtreeIndex;
		node.remove(value, subtreeIndex);
	}

	// Finds the subtree and populates the value set.
	Set< Value > & find(Index const & subtreeIndex, 
		Set< Value > & results,
		bool includePartial) const
	{
		if (1 < subtreeIndex.getChildCount())
		{
			return node.find(subtreeIndex, 0, results, includePartial);
		}
		Index index(subtreeIndex);
		index.stepToChild();
		assert(1 < index.getChildCount());
		return node.find(index, 0, results, includePartial);
	}

	bool find(Index const & subtreeIndex,
		bool includePartial) const
	{
		if (1 < subtreeIndex.getChildCount())
		{
			return node.find(subtreeIndex, 0, includePartial);
		}
		Index index(subtreeIndex);
		index.stepToChild();
		assert(1 < index.getChildCount());
		return node.find(index, 0, includePartial);
	}
	
	// Returns true if the value is found at the index.
	bool find(Index const & subtreeIndex, Value value,
		bool includePartial) const
	{
		if (1 < subtreeIndex.getChildCount())
		{
			return node.find(subtreeIndex, 0, value, includePartial);
		}
		Index index(subtreeIndex);
		index.stepToChild();
		assert(1 < index.getChildCount());
		return node.find(index, 0, value, includePartial);
	}

	// Finds the value and populates the subtree set.
	SubtreeSet & find(Value value, SubtreeSet & results) const
	{
		Index subtreeIndex;
		assert(1 < subtreeIndex.getChildCount()); 
		return node.find(value, results, subtreeIndex);
	}

	// Returns true if the value is found in the subtree.
	bool find(Value value) const
	{
		Index subtreeIndex;
		assert(1 < subtreeIndex.getChildCount()); 
		return node.find(value, subtreeIndex);
	}

	// Visits the values in the given subtree.
	// If includePartial is true,
	// visits values in any descendant subtree as well.
	bool visit(Index const & subtreeIndex, 
		Set< Value > & results,
		FunctorInterface< bool, Value > & callback,
		bool includePartial) const
	{
		if (1 < subtreeIndex.getChildCount())
		{
			return node.visit(subtreeIndex, 0, results, callback, includePartial);
		}
		Index index(subtreeIndex);
		index.stepToChild();
		assert(1 < index.getChildCount());
		return node.visit(index, 0, results, callback, includePartial);
	}
};

#endif ///////////////////////////////////////////////////

template < typename Value >
class Pyxis::Globe::Tree::SubtreeMultimap< Value >::Test
{
	char const * const subtreeIndexName;
	
	// Returns any uncle of the index; returns the same one each time.
	static Index getSubtreeIndexUncle(Index subtreeIndex, Step & childStep)
	{
		Step const parentStep = subtreeIndex.getSteps()[
			subtreeIndex.getStepCount().offset - 2];
	
		subtreeIndex.stepToParent();
		subtreeIndex.stepToParent();
		assert(1 < subtreeIndex.getChildCount());
		for (childStep = 0;
			childStep < subtreeIndex.getChildCount();
			++childStep)
		{
			if (childStep != parentStep)
			{
				break;
			}
		}
		assert(childStep != parentStep);
		subtreeIndex.stepToChild(childStep);
		return subtreeIndex;
	}

	// Get a sibling of parent other than uncle; returns the same one each time.
	static Index getSubtreeIndexAunt(Index subtreeIndex, Step & childStep)
	{
		Step const parentStep = subtreeIndex.getSteps()[
			subtreeIndex.getStepCount().offset - 2];
	
		subtreeIndex.stepToParent();
		subtreeIndex.stepToParent();
		assert(2 < subtreeIndex.getChildCount());
		for (char childCount = subtreeIndex.getChildCount();
			childCount; )
		{
			childStep = --childCount;
			if (childStep != parentStep)
			{
				break;
			}
		}
		assert(childStep != parentStep);
		subtreeIndex.stepToChild(childStep);
		return subtreeIndex;
	}
	
	// Returns any descendant of the index; returns the same one each time.
	static Index getSubtreeIndexDescendant(Index subtreeIndex)
	{
		subtreeIndex.stepToChild();
		subtreeIndex.stepToChild();
		return subtreeIndex;
	}
	
	// Returns the grandparent of the index.
	static Index getSubtreeIndexGrandparent(Index const & subtreeIndex)
	{
		assert(1 < subtreeIndex.getStepCount().offset &&
			"The index level must be 2 or higher.");
		return Index(subtreeIndex, Level(subtreeIndex.getStepCount().offset - 2));
	}

public:

	Test() : subtreeIndexName("100040") {}

	operator bool() const
	{
		std::string valueA("valueA");
		std::string valueB("valueB");
		std::string valueC("valueC");

		Index subtreeIndex(subtreeIndexName);
		assert(2 < subtreeIndex.getChildCount() &&
			"This test will only work with an index that has more than 2 children.");
		char const childCount = subtreeIndex.getChildCount();

		// Create supporting test data.
		Index const subtreeIndexDescendant(getSubtreeIndexDescendant(subtreeIndex));
		Index const subtreeIndexGrandparent(getSubtreeIndexGrandparent(subtreeIndex));
		Step uncleStep;
		Index const subtreeIndexUncle(getSubtreeIndexUncle(subtreeIndex, uncleStep));
		Step auntStep;
		Index const subtreeIndexAunt(getSubtreeIndexAunt(subtreeIndex, auntStep));

		// Create empty.
		SubtreeMultimap map;
		if (!map.getIsEmpty())
		{
			return PYXIS__ASSERT___FAIL(
				"The default constructed map was not empty.");
		}
		
		// Insert.
		{
			// Insert a node with more than 2 children.
			std::cout << "+[" << subtreeIndex << "]=>" << subtreeIndexName << std::endl;
			map.insert(subtreeIndex, subtreeIndexName);
			std::cout << map << std::endl;

			// Find the values for the index and verify.
			{
				Set< std::string > results;
				if (map.find(subtreeIndex, results, true).getCount() != 1) return PYXIS__ASSERT___FAIL("Insertion failed.");
				if (!results.find(subtreeIndexName)) return PYXIS__ASSERT___FAIL("Retrieval failed.");
			}

			// Find the indices for the value and verify.
			{
				SubtreeSet results;
				if (map.find(subtreeIndexName, results).getPairs().getCount() != 1) return PYXIS__ASSERT___FAIL("Insertion failed.");
				if (!results.find(subtreeIndex)) return PYXIS__ASSERT___FAIL("Retrieval failed.");
			}

			// Negative test (not all children have the value):
			{
				// Map all the children, but two, to value A.
				for (Step childStep = 2; childStep < childCount; ++childStep)
				{
					subtreeIndex.stepToChild(childStep);
					std::cout << "+[" << subtreeIndex << "]=>" << valueA << std::endl;
					map.insert(subtreeIndex, valueA);
					subtreeIndex.stepToParent();
				}
				std::cout << map << std::endl;

				// Verify that they're there, and that the 2 aren't.
				for (Step childStep = 0; childStep < 2; ++childStep)
				{
					subtreeIndex.stepToChild(childStep);
					Set< std::string > results;
					if (map.find(subtreeIndex, results, true).getCount() != 1) return PYXIS__ASSERT___FAIL("");
					if (!results.find(subtreeIndexName)) return PYXIS__ASSERT___FAIL("");
					subtreeIndex.stepToParent();
				}
				for (Step childStep = 2; childStep < childCount; ++childStep)
				{
					subtreeIndex.stepToChild(childStep);
					Set< std::string > results;
					if (map.find(subtreeIndex, results, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
					if (!results.find(subtreeIndexName)) return PYXIS__ASSERT___FAIL("");
					if (!results.find(valueA)) return PYXIS__ASSERT___FAIL("");
					subtreeIndex.stepToParent();
				}

				// Map one of the remaining children to value B.
				subtreeIndex.stepToChild();
				std::cout << "+[" << subtreeIndex << "]=>" << valueB << std::endl;
				map.insert(subtreeIndex, valueB);
				std::cout << map << std::endl;
				subtreeIndex.stepToParent();

				// Verify that it's there, as well as the value A nodes.
				{
					Step childStep = 0;
					subtreeIndex.stepToChild(childStep);
					Set< std::string > results;
					if (map.find(subtreeIndex, results, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
					if (!results.find(subtreeIndexName)) return PYXIS__ASSERT___FAIL("");
					if (!results.find(valueB)) return PYXIS__ASSERT___FAIL("");
					subtreeIndex.stepToParent();
				}
				{
					Step childStep = 1;
					subtreeIndex.stepToChild(childStep);
					Set< std::string > results;
					if (map.find(subtreeIndex, results, true).getCount() != 1) return PYXIS__ASSERT___FAIL("");
					if (!results.find(subtreeIndexName)) return PYXIS__ASSERT___FAIL("");
					subtreeIndex.stepToParent();
				}
				for (Step childStep = 2; childStep < childCount; ++childStep)
				{
					subtreeIndex.stepToChild(childStep);
					Set< std::string > results;
					if (map.find(subtreeIndex, results, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
					if (!results.find(subtreeIndexName)) return PYXIS__ASSERT___FAIL("");
					if (!results.find(valueA)) return PYXIS__ASSERT___FAIL("");
					subtreeIndex.stepToParent();
				}

				// Map the remaining empty child to value A.
				subtreeIndex.stepToChild(1);
				std::cout << "+[" << subtreeIndex << "]=>" << valueA << std::endl;
				map.insert(subtreeIndex, valueA);
				std::cout << map << std::endl;
				subtreeIndex.stepToParent();

				// Confirm that it was inserted, and that consolidation didn't happen.
				if (map.find(subtreeIndex, valueA, false)) return PYXIS__ASSERT___FAIL("");
				{
					Step childStep = 0;
					subtreeIndex.stepToChild(childStep);
					Set< std::string > results;
					if (map.find(subtreeIndex, results, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
					if (!results.find(subtreeIndexName)) return PYXIS__ASSERT___FAIL("");
					if (!results.find(valueB)) return PYXIS__ASSERT___FAIL("");
					subtreeIndex.stepToParent();
				}
				for (Step childStep = 1; childStep < childCount; ++childStep)
				{
					subtreeIndex.stepToChild(childStep);
					Set< std::string > results;
					if (map.find(subtreeIndex, results, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
					if (!results.find(subtreeIndexName)) return PYXIS__ASSERT___FAIL("");
					if (!results.find(valueA)) return PYXIS__ASSERT___FAIL("");
					subtreeIndex.stepToParent();
				}
			}

			// Value consolidation test.
			{
				// Map all the children, but one, to value B.  (The first element is already set.)
				for (Step childStep = 2; childStep < childCount; ++childStep)
				{
					subtreeIndex.stepToChild(childStep);
					std::cout << "+[" << subtreeIndex << "]=>" << valueB << std::endl;
					map.insert(subtreeIndex, valueB);
					subtreeIndex.stepToParent();
				}
				std::cout << map << std::endl;

				// Verify that the inserted values are there, as well as values already inserted.
				{
					subtreeIndex.stepToChild(0);
					Set< std::string > results;
					if (map.find(subtreeIndex, results, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
					if (!results.find(subtreeIndexName)) return PYXIS__ASSERT___FAIL("");
					if (!results.find(valueB)) return PYXIS__ASSERT___FAIL("");
					subtreeIndex.stepToParent();
				}
				{
					subtreeIndex.stepToChild(1);
					Set< std::string > results;
					if (map.find(subtreeIndex, results, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
					if (!results.find(subtreeIndexName)) return PYXIS__ASSERT___FAIL("");
					if (!results.find(valueA)) return PYXIS__ASSERT___FAIL("");
					subtreeIndex.stepToParent();
				}
				for (Step childStep = 2; childStep < childCount; ++childStep)
				{
					subtreeIndex.stepToChild(childStep);
					Set< std::string > results;
					if (map.find(subtreeIndex, results, true).getCount() != 3) return PYXIS__ASSERT___FAIL("");
					if (!results.find(subtreeIndexName)) return PYXIS__ASSERT___FAIL("");
					if (!results.find(valueA)) return PYXIS__ASSERT___FAIL("");
					if (!results.find(valueB)) return PYXIS__ASSERT___FAIL("");
					subtreeIndex.stepToParent();
				}

				// Map the remaining child to value B.
				subtreeIndex.stepToChild(1);
				std::cout << "+[" << subtreeIndex << "]=>" << valueB << std::endl;
				map.insert(subtreeIndex, valueB);
				std::cout << map << std::endl;
				subtreeIndex.stepToParent();

				// Confirm that it was inserted, and that value consolidation happened.
				if (!map.find(subtreeIndex, valueB, false)) return PYXIS__ASSERT___FAIL("");
				{
					subtreeIndex.stepToChild(0);
					Set< std::string > results;
					if (map.find(subtreeIndex, results, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
					if (!results.find(subtreeIndexName)) return PYXIS__ASSERT___FAIL("");
					if (!results.find(valueB)) return PYXIS__ASSERT___FAIL("");
					subtreeIndex.stepToParent();
				}
				for (Step childStep = 1; childStep < childCount; ++childStep)
				{
					subtreeIndex.stepToChild(childStep);
					Set< std::string > results;
					if (map.find(subtreeIndex, results, true).getCount() != 3) return PYXIS__ASSERT___FAIL("");
					if (!results.find(subtreeIndexName)) return PYXIS__ASSERT___FAIL("");
					if (!results.find(valueA)) return PYXIS__ASSERT___FAIL("");
					if (!results.find(valueB)) return PYXIS__ASSERT___FAIL("");
					subtreeIndex.stepToParent();
				}
			}

			// Full consolidation test.
			{
				// Insert value A in the last node.  This should result in consolidation.
				subtreeIndex.stepToChild();
				std::cout << "+[" << subtreeIndex << "]=>" << valueA << std::endl;
				map.insert(subtreeIndex, valueA);
				std::cout << map << std::endl;
				subtreeIndex.stepToParent();

				// Confirm state.
				if (!map.find(subtreeIndex, valueA, false)) return PYXIS__ASSERT___FAIL("");
				for (Step childStep = 0; childStep < childCount; ++childStep)
				{
					subtreeIndex.stepToChild(childStep);
					Set< std::string > results;
					if (map.find(subtreeIndex, results, true).getCount() != 3) return PYXIS__ASSERT___FAIL("");
					if (!results.find(subtreeIndexName)) return PYXIS__ASSERT___FAIL("");
					if (!results.find(valueA)) return PYXIS__ASSERT___FAIL("");
					if (!results.find(valueB)) return PYXIS__ASSERT___FAIL("");
					subtreeIndex.stepToParent();
				}
			}

			// Insert ancestor.
			{
				// Insert valueB.
				std::cout << "+[" << subtreeIndexGrandparent << "]=>" << valueB << std::endl;
				map.insert(subtreeIndexGrandparent, valueB);
				std::cout << map << std::endl;

				// Confirm.
				if (!map.find(subtreeIndexGrandparent, valueB, false)) return PYXIS__ASSERT___FAIL("");
				if (map.find(subtreeIndexGrandparent, valueA, false)) return PYXIS__ASSERT___FAIL("");
				Set< Value > values;
				if (map.find(subtreeIndexGrandparent, values, true).getCount() != 3) return PYXIS__ASSERT___FAIL("");
			}
			
			// Insert another, not in lineage, with same value.
			{
				// Insert valueA.
				std::cout << "+[" << subtreeIndexUncle << "]=>" << valueA << std::endl;
				map.insert(subtreeIndexUncle, valueA);
				std::cout << map << std::endl;
				
				// Confirm.
				{
					Set< Value > values;
					if (map.find(subtreeIndexUncle, values, true).getCount() != 2) return PYXIS__ASSERT___FAIL("");
				}
				if (!map.find(subtreeIndexUncle, valueA, false)) return PYXIS__ASSERT___FAIL("");
				{
					SubtreeSet subtreeIndices;
					if (map.find(valueA, subtreeIndices).getPairs().getCount() != 2) return PYXIS__ASSERT___FAIL("");
					if (!subtreeIndices.find(subtreeIndexUncle)) return PYXIS__ASSERT___FAIL("");
					if (!subtreeIndices.find(subtreeIndex)) return PYXIS__ASSERT___FAIL("");
				}
			}
		}
		
		// Find.
		{
			// By index: implicit value, explicit leaf.
			{
				// Find subtreeIndex
				if (!map.find(subtreeIndex, true)) return PYXIS__ASSERT___FAIL("");
			
				// Find values at subtreeIndex
				Set< Value > values;
				map.find(subtreeIndex, values, true);
				
				// Verify valueA, valueB, and subtreeIndexName.
				if (values.getCount() != 3) return PYXIS__ASSERT___FAIL("");
			}
			
			// By index: implicit value, explicit leaf descendant.
			{
				// Find subtreeIndex descendant.
				if (!map.find(subtreeIndexDescendant, true)) return PYXIS__ASSERT___FAIL("");

				// Find values at subtreeIndex descendant.
				// Verify valueA, valueB, and subtreeIndexName.
				Set< Value > values;
				if (map.find(subtreeIndex, values, true).getCount() != 3) return PYXIS__ASSERT___FAIL("");
			}
			
			// Negative by index.
			{
				// Boolean form
				if (!map.find(subtreeIndexAunt, true)) return PYXIS__ASSERT___FAIL("");
				
				// Results form
				Set< Value > values;
				if (map.find(subtreeIndexAunt, values, true).getCount() != 1) return PYXIS__ASSERT___FAIL("");
				if (!values.find(valueB)) return PYXIS__ASSERT___FAIL("");
			}
			
			// By value.
			{
				if (!map.find(subtreeIndexName)) return PYXIS__ASSERT___FAIL("");
				if (!map.find(valueA)) return PYXIS__ASSERT___FAIL("");
				if (!map.find(valueB)) return PYXIS__ASSERT___FAIL("");

				{
					SubtreeSet subtreeIndices;
					if (map.find(subtreeIndexName, subtreeIndices).getPairs().getCount() != 1) return PYXIS__ASSERT___FAIL("");
				}
				{
					SubtreeSet subtreeIndices;
					if (map.find(valueA, subtreeIndices).getPairs().getCount() != 2) return PYXIS__ASSERT___FAIL("");
				}
				{
					SubtreeSet subtreeIndices;
					if (map.find(valueB, subtreeIndices).getPairs().getCount() != 1) return PYXIS__ASSERT___FAIL("");
				}
			}
			
			// Negative by value.
			{
				if (map.find(valueC)) return PYXIS__ASSERT___FAIL("");

				SubtreeSet subtreeIndices;
				if (!map.find(valueC, subtreeIndices).getIsEmpty()) return PYXIS__ASSERT___FAIL("");
			}
		}
		
		// Copy.
		{
			SubtreeMultimap copy(map);
			Set< Value > values;
			Index subtreeRootIndex;
			{
				Set< Value > copyValues;
				map.find(subtreeRootIndex, values, true);
				copy.find(subtreeRootIndex, copyValues, true);
				if (values != copyValues) return PYXIS__ASSERT___FAIL("");
			}
#if 0 // TODO: Add these when additional equality is implemented.
			for (typename Set< Value >::Elements elements(values); !elements.getIsEmpty(); elements.popFront())
			{
				Value value = elements.getFront();
				SubtreeSet subtrees;
				SubtreeSet copySubtrees;
				if (map.find(value, subtrees, true) != copy.find(value, copySubtrees, true)) return PYXIS__ASSERT___FAIL("");
			}
			if (copy != map) return PYXIS__ASSERT___FAIL("");
#endif
		}

		// Remove.
		{
			// Negative, with sibling.
			{
				// Remove valueA from aunt.
				std::cout << "-[" << subtreeIndexAunt << "]=>" << valueA << std::endl;
				map.remove(subtreeIndexAunt, valueA);
				std::cout << map << std::endl;

				// Verify that there is no change.
				SubtreeSet subtreeIndices;
				if (map.find(valueA, subtreeIndices).getPairs().getCount() != 2) return PYXIS__ASSERT___FAIL("");
				if (!subtreeIndices.find(subtreeIndex)) return PYXIS__ASSERT___FAIL("");
			}

			// Value deconsolidation.
			{
				// Remove valueB from aunt.
				std::cout << "-[" << subtreeIndexAunt << "]=>" << valueB << std::endl;
				map.remove(subtreeIndexAunt, valueB);
				std::cout << map << std::endl;
				
				// Verify addition of valueB to its siblings, and removal from parent.
				Index index(subtreeIndex);

				// Step to the parent.
				// If it has only 1 child, step to its parent.
				index.stepToParent();
				char childCount = index.getChildCount();
				assert(childCount);
				if (1 == childCount)
				{
					index.stepToParent();
					childCount = index.getChildCount();
					assert(1 < childCount);
				}

				if (map.find(index, valueB, false)) return PYXIS__ASSERT___FAIL("");
				SubtreeSet subtreeIndices;
				if (map.find(valueB, subtreeIndices).getPairs().getCount() != size_t(childCount - 1))
				{
					return PYXIS__ASSERT___FAIL("");
				}
				do
				{
					Step const childStep = --childCount;
					index.stepToChild(childStep);
					if (childStep == auntStep)
					{
						if (map.find(index, valueB, false)) return PYXIS__ASSERT___FAIL("");
					} else
					{
						if (!map.find(index, valueB, false)) return PYXIS__ASSERT___FAIL("");
					}
					index.stepToParent();
				} while (childCount);
			}

			// Full deconsolidation.
			{
				// Remove valueA from subtreeIndex descendant.
				Index subtreeIndexChild(subtreeIndex);
				char childCount = subtreeIndex.getChildCount();
				assert(1 < childCount);
				Step childStep = childCount - 1;
				subtreeIndexChild.stepToChild(childStep);
				std::cout << "-[" << subtreeIndexChild << "]=>" << valueA << std::endl;
				map.remove(subtreeIndexChild, valueA);
				std::cout << map << std::endl;

				// Verify removal of valueA at subtreeIndex decendant, and addition at all its siblings.
				if (map.find(subtreeIndex, valueA, false)) return PYXIS__ASSERT___FAIL("");
				if (map.find(subtreeIndexChild, valueA, true)) return PYXIS__ASSERT___FAIL("");
				SubtreeSet subtreeIndices;
				if (map.find(valueA, subtreeIndices).getPairs().getCount() != (size_t)childCount)
				{
					return PYXIS__ASSERT___FAIL("");
				}
				do
				{
					Step const step = --childCount;
					subtreeIndexChild.stepToParent();
					subtreeIndexChild.stepToChild(step);
					if (step == childStep)
					{
						if (map.find(subtreeIndexChild, valueA, true)) return PYXIS__ASSERT___FAIL("");
					} else
					{
						if (!map.find(subtreeIndexChild, valueA, false)) return PYXIS__ASSERT___FAIL("");
					}
				} while (childCount);
			}

			// Remove leaf.
			{
				// Remove valueA from a child.
				Index subtreeIndexChild(subtreeIndex);
				subtreeIndexChild.stepToChild();
				std::cout << "-[" << subtreeIndexChild << "]=>" << valueA << std::endl;
				map.remove(subtreeIndexChild, valueA);
				std::cout << map << std::endl;
				
				// Verify single removal.
				if (map.find(subtreeIndexChild, valueA, true)) return PYXIS__ASSERT___FAIL("");
				char const childCount = subtreeIndex.getChildCount();
				for (Step step = 1; step < childCount - 1; ++step)
				{
					subtreeIndexChild.stepToParent();
					subtreeIndexChild.stepToChild(step);
					if (!map.find(subtreeIndexChild, valueA, false)) return PYXIS__ASSERT___FAIL("");
				}
			}
			
			// Remove ancestor with no differing values below.
			{
				// Remove valueA from subtreeIndex.
				std::cout << "-[" << subtreeIndex << "]=>" << valueA << std::endl;
				map.remove(subtreeIndex, valueA);
				std::cout << map << std::endl;
				
				// Verify that it only exists at uncle.
				{
					SubtreeSet subtreeIndices;
					if (map.find(valueA, subtreeIndices).getPairs().getCount() != 1) return PYXIS__ASSERT___FAIL("");
					if (!subtreeIndices.find(subtreeIndexUncle)) return PYXIS__ASSERT___FAIL("");
				}
				
				// Remove valueA.
				std::cout << "-[]=>" << valueA << std::endl;
				map.remove(valueA);
				std::cout << map << std::endl;
				
				// Verify that valueA is removed entirely from tree.
				if (map.find(valueA)) return PYXIS__ASSERT___FAIL("");
				if (!map.find(subtreeIndexName)) return PYXIS__ASSERT___FAIL("");
				if (!map.find(valueB)) return PYXIS__ASSERT___FAIL("");
			}

			// Remove ancestor with differing values below.
			{
				// Remove "subtreeIndexName" from subtreeIndex.
				std::cout << "-[" << subtreeIndex << "]=>" << subtreeIndexName << std::endl;
				map.remove(subtreeIndex, subtreeIndexName);
				std::cout << map << std::endl;
				
				// Verify that valueB remains at parent.
				if (map.find(subtreeIndexName)) return PYXIS__ASSERT___FAIL("");
				SubtreeSet subtreeIndices;
				if (map.find(valueB, subtreeIndices).getPairs().getCount() <= 1) return PYXIS__ASSERT___FAIL("");
			}

			// Remove ancestor that clears the map.
			{
				// Remove valueB from root.
				std::cout << "-[]=>" << valueB << std::endl;
				map.remove(valueB);
				std::cout << map << std::endl;

				// Verify that the tree is cleared.
				if (!map.getIsEmpty()) return PYXIS__ASSERT___FAIL("");
			}
		}

		// Merge.
		// TODO: Add automated verification
		{
			std::cout << "+[003]=>" << valueA << std::endl;
			map.insert(Index("003"), valueA);
			std::cout << map << std::endl;

			std::cout << "+[00401]=>" << valueA << std::endl;
			map.insert(Index("00401"), valueA);
			std::cout << map << std::endl;

			std::cout << "-[003]=>" << valueA << std::endl;
			map.remove(Index("003"), valueA);
			std::cout << map << std::endl;
		}

		// Captured bug found in real-world test.
		/*
		({}{0}
		 [0]{0}{}
		 [1]{}{0}
		  (0)
		   [0]{0}{}
		   [1]{0}{}
		   [2]{}{0}
			(0)
			 [0]
			  [0]
			   [0]{0}{}
			   [1]{0}{}
			   [2]{0}{}
			   [3]{0}{}
			   [4]{0}{}
			   [5]{}{0}
				(0)
				 [0]{}{}
				 [1]{0}{}
				 [2]{0}{}
				 [3]{0}{}
				 [4]{0}{}
				 [5]{0}{}
				 [6]{0}{}
		   [3]{}{}
		   [4]{}{}
		   [5]{}{})
		   
		+[102001]{0}

		*/
		// TODO: Add automated verification
		std::cout << "CLEAR" << std::endl;
		map.setIsEmpty();
		std::cout << map << std::endl;
		{
			// Set it up.
			{
				std::string indexName("0"); 
				std::cout << "+[" << indexName << "]=>" << valueA << std::endl;
				map.insert(Index(indexName.c_str()), valueA);
				std::cout << map << std::endl;
			}
			{
				std::string indexName("100"); 
				std::cout << "+[" << indexName << "]=>" << valueA << std::endl;
				map.insert(Index(indexName.c_str()), valueA);
				std::cout << map << std::endl;
			}
			{
				std::string indexName("101"); 
				std::cout << "+[" << indexName << "]=>" << valueA << std::endl;
				map.insert(Index(indexName.c_str()), valueA);
				std::cout << map << std::endl;
			}
			{
				std::string indexName("102000"); 
				std::cout << "+[" << indexName << "]=>" << valueA << std::endl;
				map.insert(Index(indexName.c_str()), valueA);
				std::cout << map << std::endl;
			}
			{
				std::string indexName("102000500"); 
				std::cout << "-[" << indexName << "]=>" << valueA << std::endl;
				map.remove(Index(indexName.c_str()), valueA);
				std::cout << map << std::endl;
			}

			// Insert the problem.
			{
				std::string indexName("102001"); 
				std::cout << "+[" << indexName << "]=>" << valueA << std::endl;
				map.insert(Index(indexName.c_str()), valueA);
				std::cout << map << std::endl;
			}
		}

		return true;
	}
};

#endif
