//#include "spherical_cell.h"

#include <cassert>
#include <iostream>
#include <vector>

/*

TODO:

-	Adjust for negative resolutions:
	*	Resolutions:
		-	...
		-	-2 = 3/5 icosahedron
		-	-1 = 1/4 icosahedron
		-	0 = dodecahedron
		-	1 = truncated icosahedron
		-	...

	-	Expose in interface; possibly all the way down
		-	


---

-	Consider: at TI, moving from equ. P or H, we result in either a 
	correct P, correct H, or incorrect H (non-zero vertex grandchild of non-polar 1/4).

	=================================================

		
	private:
	
		// A child of a cell.
		// Exposes a local operation interface.
		template <char unsigned zero> struct Child
		{
			virtual ~Child() {}

			char unsigned GetOffset() const
			{
				return offset;
			}

			virtual char unsigned Increment()
			{
				if (!offset) return 0;
				if (offset == zero)
				{
					offset = 1;
				} else
				{
					++offset;
				}
				return 4;
			}
			virtual char unsigned Decrement()
			{
				if (!offset) return 0;
				if (offset == 1)
				{
					offset = zero;
				} else
				{
					--offset;
				}
				return 2;
			}

			virtual char unsigned GetHalfIncrement() const = 0;
			virtual char unsigned GetHalfDecrement() const = 0;
			
		protected:

			Child() : offset(0) {}
			
			SetOffset(char unsigned o)
			{
				offset = o; 
			}

			char unsigned offset;
		};
		
		// A child of a hexagon.
		struct HexagonChild : Child<6>
		{
			enum Rotation
			{
				one = 1, two, three, four, five, zero
			};

			HexagonChild() : offset(0) {}
			HexagonChild(Rotation const o) : offset(o) {}

			SetOffset(Rotation const o)
			{
				assert(o && o <= zero);
				offset = o; 
			}

			virtual char unsigned GetHalfIncrement() const
			{
			}
			virtual char unsigned GetHalfDecrement() const
			{
			}
		};

		// A child of a pentagon.
		struct PentagonChild : Child<5>
		{
			enum Rotation
			{
				one = 1, two, three, four, zero
			};

			PentagonChild() : offset(0) {}
			PentagonChild(Rotation const o) : offset(o) {}

			SetOffset(Rotation const o)
			{
				assert(o && o <= zero);
				offset = o; 
			}

			virtual char unsigned Increment()
			{
				char unsigned edge = Child<5>::Increment();
			
				// TODO: Special normalization for D and TI
				
				return edge;
			}
			virtual char unsigned Decrement()
			{
				char unsigned edge = Child<5>::Decrement();

				// TODO: Special normalization for D and TI

				return edge;
			}

			virtual char unsigned GetHalfIncrement() const
			{
			}
			virtual char unsigned GetHalfDecrement() const
			{
			}
		};

		HexagonChild::Rotation StepIn(size_t const index)
		{
			assert(IsCentroidOffset(index));

		}

		HexagonChild::Rotation Step(size_t const index, HexagonChild::Rotation edge)
		{
			// TODO: Asserts

			static HexagonChild::Rotation (* const step[])(size_t const index) =
			{
				&StepOut,
				&HalfIncrement,
				&Increment,
				&StepIn,
				&Decrement,
				&HalfDecrement,
				&StepOut
			};
	
			// Step.
			if (IsCentroid(index))
			{
				// Step from centroid.
				return path[index]->SetOffset(edge);
			} else
			{
				// Step from vertex.
				return step[edge](index);
			}
		}		
	
	private:
	
		bool rotated;

		std::vector<PentagonChild> pentagonChildren;
		std::vector<HexagonChild> hexagonChildren;

		std::vector<Child *> path;
	};


-----------


-	Decide equatorial pentagon orientation: should they be reverse?

-	Finish implementing ToAdjacent.
	-	Work out "from vertex centroid" case.
		-	Hexagon:

						6

				1	  1	  2		5
					  
					6		3
		
				2	  5   4		4
		
						3


				-	Smallest angle:
					1, 4	(1 mod 3)
				-	Right angle:
					3, 6	(0 mod 3)
				-	Largest angle:
					5, 2	(2 mod 3)



				Negating the outer ones gives uniform rotation between lattices:
				
				
						6

				5	  1	  2		1
					  
					6		3
		
				4	  5   4		2
		
						3




		-	Pentagon
		
		
		
		
		

	-	Audit all the existing logic and comments.

	-	Fill in "Rotate Out" case.
		Factor it out into a separate function if possible.

	-	...

-	Test.
	-	Implement Cell::Iterate(...)

	-	Implement tests that go to each cell, then go back and forth
		from each direction, and test.

	-	Run and debug.

-	Back it up.
-	Remove dead crap.

-	Document everything in file, including design.

-	Write Cell and Index pages on the wiki.
	-	Goals:
		-	Any cell 

-	Optimize.
	-	Less code.
		-	Replace all the "if (IsHexagon())" stuff with polymorphic approach?
		
			struct Cell
			{
				typedef Modulo<6> Rotation;

				struct INode
				{
					// Define what would go in here
				};
				
				struct Hexagon : INode
				{
					// Implement
				};
				
				struct Pentagon : INode
				{
					// Implement
				};

			private:

				Rotation ToAdjacent(size_t resolution, Rotation edge)
				{
					// Modify the code to be polymorphic.
				}
			
				// Owned.
				std::vector<INode *> path; 
			};
			
		
			-	Cell is an owned pointer list; each is a subclass of ICell:
				Hexagon or Pentagon.
	
	-	More performance.

*/

/*

-	Design:

	-	Rotation:
		-	Zero points back to centroid.
			-	Pentagon: 5
			-	Hexagon: 6


	-	Refine terms.
		-	Centroid Offset:
			-	An offset from a centroid.  Can be one of the following:
				-	null (for centroid)
				-	Pentagon::Rotation (if pentagon)
				-	Hexagon::Rotation (if hexagon)

		-	Cell Offset:
			-	An offset from a cell, comprised as a series of centroid offsets from root.

		-	Cell Offset Index:
			-	The 0-based index of a centroid offset within a cell offset.

		-	Cell Offset Resolution:
			-	The count of centroid offsets within a cell offset.
			
		-	Cell:
			-	Each vertex is labelled with a Hexagon::Rotation (if hexagonal)
				or Pentagon::Rotation (if pentagonal).
			-	Each vertex child is labelled for the vertex at its center.
			-	The edges are named as follows:
				-	Centroid centroid:
					-	Each edge is named for the vertex child on
						the other side of the edge.
				-	Vertex centroid:
					-	...
				-	Vertex:
					-	...

	*	Resolutions:
		-	0 = sphere
		-	1 = 3/4 icosahedron.  In public form, ToAdjacent goes to res 2 of other pole.
		-	2 = 1/4 icosahedron.  In public form, ToAdjacent goes to res 1 of other pole.
		-	3 = dodecahedron

	*	Elements:
		-	[0] = pole; Rotation::zero or Rotation::opposite.  3/4 icos.
		-	[1] = pole; null.  1/4 icos.
		-	[2] = dodecahedron.
		-	[3] = TI.

	*	Every pentagon has the same edge and vertex numbering:
		edges 1 to 5, ordered, where vertex connecting edges a and b is (a+b)/2 in F5.
		
	*	Every hexagon has the same edge and vertex numbering:
		edges 0 to 5, ordered, where vertex connecting edges a and b, where a < b, is either
		(b+1) or (a-1), depending on lattice.

	*	Cell types:
		-	Hexagon:
			-	Centroid:
				-	Children:
					-	centroid hexagon
					-	6 vertex hexagons: vertices 0 to 5
			-	Vertex:
				-	Children:
					-	centroid hexagon

		-	Pentagon:
			-	Centroid:
				-	Children:
					-	centroid pentagon
					-	5 vertex hexagons: vertices 1 to 5
			-	Vertex:
				-	Children:
					-	centroid pentagon
					-	1 vertex hexagon: vertex 3

	*	Hexagon spawned by pentagon: direction 3 points back to pentagon.

	*	Offset:
		-	Left-aligned
		-	Series of directions; first one corresponds to sign:
			vertex0 = same; vertex3 = other; all other = denormalized,
			normalize to vertex3.

	-	Operations:
		-	Add (index1 + offset = index2)
			-	If adding to an index of the opposite sign, changes the sign.
			-	If longer than index, drills down.
		-	Subtract (index2 - index1 = offset)
			-	Get the offset from index1 to index2.
			-	Not commutative.
		-	Negate (offset)
			-	Flip the sign of the offset.
		-	Negate (index)
			-	Flip the sign (pole) of the index.
			-	Same as adding negative offset with empty path.

-	Rationale:

	*	Vertex pentagon has vertex child 3 so that it lines up with children.

	*	Numbering of hexagonal directions: can be numbered with rotations 
		(a) relative to spawning pentagon directions, or
		(b) relative to spawning centroid directions.

		-	Option (a) results in uniform directions within hexagon at all resolutions.
		-	Option (b) results in radial numbering (e.g. direction 3 leads to a centroid
			and direction 0 leads away).
			-	Not "orthogonal" (Perry's term).

	*	Offsets are left-aligned so that a specific offset of the index
		can always refer to a pentagon, so the meaning will be preserved.

*/

#if 1

namespace PYXIS
{
	namespace DERM
	{
		namespace Geometry
		{
			struct Cell
			{
				enum Rotation
				{
					one = 1, two, three, four, five, six
				};

				struct Offset
				{
					Offset(bool n)
					: isNegative(n), subOffsets()
					{
					}

					bool IsNegative() const
					{
						return isNegative;
					}
					
					void Negate()
					{
						isNegative = !isNegative;
					}
					
					size_t Resolution() const
					{
						return subOffsets.size();
					}
					
					// Throws if none to pop.
					void Pop()
					{
						subOffsets.pop_back(); // Throws.
					}

					// Push a null sub-offset.
					// Throws if out of memory.
					void Push() 
					{
						subOffsets.push_back(0); // Throws.
					}
					
					// Push a non-null sub-offset.
					// Throws if out of memory.
					void Push(Rotation const subOffset)
					{
						subOffsets.push_back(subOffset); // Throws.
					}
					
					// If there is a non-null sub-offset at the top
					// (the most-recently pushed), returns true; else, returns false.
					// Throws if empty.
					bool Top() const
					{
						return 0 != subOffsets.back(); // Throws.
					}
					
					// If there is a non-null sub-offset at the top 
					// (the most-recently pushed), populates
					// the argument and returns true; otherwise, returns false.
					// Throws if empty.
					bool Top(Rotation & subOffset) const
					{
						int unsigned const top = subOffsets.back(); // Throws.
						if (!top) return false;
						subOffset = (Rotation)top;
						return true;
					}
					
					// If there is a non-null sub-offset at the index, returns true;
					// otherwise, returns false.
					// Throws if the index is out of range.
					bool Get(size_t const index) const
					{
						return 0 != subOffsets.at(index); // Throws.
					}
					
					// If there is a non-null sub-offset at the index, populates
					// the argument and returns true; otherwise, returns false.
					// Throws if the index is out of range.
					bool Get(size_t const index, Rotation & subOffset) const
					{
						int unsigned const top = subOffsets.at(index); // Throws.
						if (!top) return false;
						subOffset = (Rotation)top;
						return true;
					}
					
					// Sets the sub-offset at the index to null.
					// Throws if the index is out of range.
					void Set(size_t const index)
					{
						subOffsets.at(index) = 0; // Throws.
					}
					
					// Sets the sub-offset at the index to the specified non-null
					// sub-offset.
					// Throws if the node is out of range.
					void Set(size_t const index, Rotation const subOffset)
					{
						subOffsets.at(index) = subOffset; // Throws.
					}
					
				private:

					bool isNegative;

					// A list of nullable sub-offsets at successive resolutions.
					std::vector<char unsigned> subOffsets;
				};

				// Constructs the root cell.
				Cell(bool n)
				: pentagonCount(0), offset(n)
				{
				}

				bool IsNegative() const
				{
					return offset.IsNegative();
				}
				
				void Negate()
				{
					offset.Negate();
				}

				// Returns the resolution of the cell.
				//	0 = whole; neither pentagon or hexagon.
				//	1 = 3/4 icosahedron; centroid.
				//	2 = 1/4 icosahedron: centroid or vertex.
				//	3 = dodecahedron: centroid.
				//	4 = truncated icosahedron: vertex child hexagon or centroid child pentagon.
				size_t Resolution() const
				{
					return offset.Resolution();
				}

				// Returns true if this cell is a hexagon.
				bool IsHexagon() const
				{
					size_t const resolution = Resolution();
					return resolution && IsHexagon(resolution - 1);
				}
				
				// Returns true if this cell is offset from a centroid,
				// or false if it is a centroid.
				bool IsCentroidOffset() const
				{
					return offset.Resolution() && offset.Top();
				}

				// Moves to parent.
				// Throws if empty.
				void ToParent()
				{
					offset.Pop(); // Throws.

					// Update pentagon count.
					size_t const resolution = Resolution();
					if (pentagonCount > resolution)
					{
						pentagonCount = resolution;
					}
				}

				// Moves to centroid child.
				// Throws if out of memory.
				void ToChild()
				{
					offset.Push(); // Throws.
				}
				
				// Moves to vertex child.
				// Throws if out of memory.
				// Returns false if there is not a child in the given direction.
				bool ToChild(Rotation const rotation) 
				{
					(void)rotation;
					// ...
					return false;
				}

				// Adds an offset to the index.
				// Left-aligned.
				// Steps in each rotation direction.
				// For resolution 1 and 2: same as moving to adjacent,
				// then pushing/popping to result in the same resolution.
				void Add(Offset const & addend)
				{
					(void)addend;
					// ...
				}
				
				// Subtracts a cell, resulting in an offset.
				// Left-aligned.
				void Subtract(Cell const & subtrahend, Offset & difference) const
				{
					(void)subtrahend;
					(void)difference;
					// ...
				}
				
				Rotation Step(Rotation edge)
				{
					if (!IsHexagon() && !Pentagon::IsEdge(edge)) return edge;

					// TODO: Handle low resolutions

					return Step(Resolution(), edge);
				}
			
			private:

				struct Hexagon
				{
					static Rotation Zero()
					{
						return six;
					}

					static Rotation Opposite()
					{
						return three;
					}

					static Rotation Reverse(Rotation rotation)
					{
						int const reverse = Zero() - rotation;
						return reverse? (Rotation)reverse: Zero();
					}

					static Rotation StepOut(Cell & cell, size_t const index);

					static Rotation StepIn(Cell & cell, size_t const index);
					
					static Rotation HalfDecrement(Cell & cell, size_t const index);

					static Rotation HalfIncrement(Cell & cell, size_t const index);

					static Rotation Decrement(Cell & cell, size_t const index);

					static Rotation Increment(Cell & cell, size_t const index);
					
					// Step from cell, which is a vertex child of a hexagon.
					static Rotation StepFromVertexChild(Cell & cell, size_t const index, Rotation edge)
					{
						static Rotation (* const stepFromVertexChild[])(Cell & cell, size_t const index) =
						{
							&StepIn,
							&Decrement,
							&HalfDecrement,
							&StepOut,
							&HalfIncrement,
							&Increment,
							&StepIn
						};
						
						return stepFromVertexChild[edge](cell, index);
					}
					
					// Step from cell, which is a child of a hexagon.
					static Rotation StepFromChild(Cell & cell, size_t const index, Rotation edge)
					{
						assert(3 < index);
						assert(cell.IsHexagon(index - 1));
						
						if (cell.IsCentroidOffset(index)) // Cell is vertex.
						{
							return StepFromVertexChild(cell, index, edge);
						}

						// Cell is centroid.
						cell.SetCentroidOffset(index, edge);
						edge = Zero();
						
						// Normalize for centroid: vertex must have centroid parent.
						if (cell.IsCentroidOffset(index - 1))
						{
							assert(cell.IsHexagon(index));
							
							// TODO: Find the correct parent and set.
						}
						
						return edge;
					}
					
				private:
				
					Hexagon();
				};

				struct Pentagon
				{
					static enum Rotation Zero()
					{
						return five;
					}

					static bool IsEdge(Rotation rotation)
					{
						return rotation != six;
					}

					static Rotation Reverse(Rotation rotation)
					{
						assert(IsEdge(rotation));

						int const reverse = Zero() - rotation;
						return reverse? (Rotation)reverse: Zero();
					}

					static Rotation StepIn(Cell & cell, size_t const index);
					
					static Rotation HalfDecrement(Cell & cell, size_t const index);

					static Rotation HalfIncrement(Cell & cell, size_t const index);

					static Rotation Decrement(Cell & cell, size_t const index);

					static Rotation Increment(Cell & cell, size_t const index);
					
					// Step from cell, which is a vertex child of a pentagon.
					static Rotation StepFromVertexChild(Cell & cell, size_t const index, Rotation edge)
					{
						static Rotation (* const stepFromVertexChild[])(Cell & cell, size_t const index) =
						{
							&StepIn,
							&Decrement,
							&HalfDecrement,
							&HalfIncrement,
							&Increment,
							&StepIn
						};
						
						assert(IsEdge(edge));
						return stepFromVertexChild[edge](cell, index);
					}

					// Step from cell, which is a child of a pentagon.
					static Rotation StepFromChild(Cell & cell, size_t const index, Rotation edge)
					{
						assert(0 < index);
						assert(IsEdge(edge));

						// Denormalize cell:
						//	-	If non-polar dodecahedron pentagon.
						switch (index)
						{
						case 2:
							{ 
								// If non-polar, denormalize the sign.
								Rotation vertex;
								if (cell.GetCentroidOffset(1, vertex))
								{
									cell.SetCentroid(1);
									cell.SetCentroidOffset(2, vertex);
									edge = Pentagon::Reverse(edge);
									cell.offset.Negate();
								}
							}
						}

						if (!cell.IsCentroidOffset(index)) // Cell is centroid.
						{
							// Move from centroid.
							cell.SetCentroidOffset(index, edge);

							// Adjust pentagon count.
							AdjustPentagonCount(cell, index);

							// Set edge.
							edge = cell.IsHexagon(index)? Hexagon::Zero(): Pentagon::Zero();

						} else // Cell is vertex.
						{
							// Step from vertex.
							edge = cell.IsHexagon(index)? 
								Hexagon::StepFromVertexChild(cell, index, edge):
								Pentagon::StepFromVertexChild(cell, index, edge);

							// Increase pentagon count until a centroid offset is hit (including this).
							AdjustPentagonCount(cell, index);
						}

						// Normalize cell:
						//	-	If non-polar dodecahedron pentagon.
						//	-	If non-polar truncated icosahedron hexagon.
						switch (index)
						{
						case 2:
							{
								// If dodecahedron face is vertex, normalize to centroid.
								Rotation vertex;
								if (cell.GetCentroidOffset(2, vertex))
								{
									assert(!cell.IsCentroidOffset(0));
									assert(!cell.IsCentroidOffset(1));

									cell.SetCentroid(2);
									cell.SetCentroidOffset(1, vertex);
									edge = Hexagon::Reverse(edge); // TODO: Revisit; in terms of Pentagon
									cell.offset.Negate();
									
									AdjustPentagonCount(cell, index);
								}
							}
							break;
						case 3:
							// If hexagon, and child of non-polar pentagon:
							if (cell.IsHexagon(index) && cell.IsCentroidOffset(1))
							{
								// Normalize the vertex.
								Rotation vertex;
								if (cell.GetCentroidOffset(3, vertex))
								{
									assert(!cell.IsCentroidOffset(0));
									assert(!cell.IsCentroidOffset(2));
									
									switch (vertex)
									{
									default:
										assert(0);

									case five:
										// No normalization required.
										break;

									case one:
									case four:
										// TODO: Go to adjacent zero vertex.
										break;

									case two:
									case three:
										// TODO: Go to centroid vertex.
										break;
									}
								}
							}
						}
						
						return edge;
					}

				private:

					static void AdjustPentagonCount(Cell & cell, size_t index)
					{
						if (index <= cell.pentagonCount)
						{
							cell.pentagonCount = index;

							size_t const resolution = cell.Resolution();
							while (!cell.IsCentroidOffset(cell.pentagonCount) && cell.pentagonCount < resolution)
							{
								++cell.pentagonCount;
							}
						}
					}
				
					Pentagon();
				};

				bool IsHexagon(size_t const index) const
				{
					return pentagonCount <= index;
				}
				
				// Returns true if it is a centroid offset, or false if it is a centroid.
				bool IsCentroidOffset(size_t const index) const
				{
					return offset.Get(index);
				}

				// Returns true if it is the given centroid offset, or false if not.
				bool IsCentroidOffset(size_t const index, Rotation const centroidOffsetToMatch) const
				{
					Rotation centroidOffset;
					return offset.Get(index, centroidOffset) && centroidOffsetToMatch == centroidOffset;
				}

				// Returns true if it is a centroid offset (and sets 'centroidOffset'
				// to the value), or false if it is a centroid.
				bool GetCentroidOffset(size_t const index, Rotation & centroidOffset) const
				{
					return offset.Get(index, centroidOffset);
				}
				
				void SetCentroid(size_t const index)
				{
					offset.Set(index);
				}

				void SetCentroidOffset(size_t const index, Rotation const centroidOffset)
				{
					offset.Set(index, centroidOffset);
				}

				Rotation Step(size_t const index, Rotation edge)
				{
					// TODO: Audit low resolutions
					// TODO: Asserts
					assert(0 < index);
					assert(IsHexagon(index) || Pentagon::IsEdge(edge));

					return IsHexagon(index - 1)?
						Hexagon::StepFromChild(*this, index, edge):
						Pentagon::StepFromChild(*this, index, edge);
				}

				friend struct Pentagon;
				friend struct Hexagon;

				size_t pentagonCount;

				Offset offset;
			};
		}
	}
}










#else

namespace PYXIS
{
	namespace Number
	{
		// Some common, simple modulo functions.
		// The 'zero' is the modulus, implemented as such, and is treated as zero.
		// Integral 0 is not used, leaving it available to indicate null
		// as desired by client code.
		template <char unsigned zero>
		struct Modulo
		{
			static char unsigned Normalize(char unsigned number)
			{
				return ((number && number <= zero) || (number %= zero))? number: zero;
			}
			
			static bool IsValid(char unsigned const number)
			{
				return number && number <= zero;
			}
			
			// Returns the additive identity element.
			static Modulo Zero()
			{
				static Modulo const z;
				return z;
			}

			// Adds the specified amount.
			// Commutative.
			static Modulo Add(Modulo const addend, Modulo const augend)
			{
				return Modulo(addend.element + augend.element);
			}

			// Generates the additive inverse.
			static Modulo Reverse(Modulo number)
			{
				number.element = zero - number.element;
				return number;
			}
			
			static Modulo Subtract(Modulo const minuend, Modulo const subtrahend)
			{
				return Add(minuend, Reverse(subtrahend));
			}
			
			// Increment by one.
			static Modulo Increment(Modulo number)
			{
				number.element = (number.element == zero)? 1: number.element + 1;
				return number;
			}
			
			// Decrement by one.
			static Modulo Decrement(Modulo number)
			{
				number.element = (number.element == 1)? zero: number.element - 1;
				return number;
			}
			
			// Generates the mininum of the rotation and its additive inverse,
			// resulting in the distance around the number wheel.
			// Commutative.
			static Modulo Distance(Modulo from, Modulo to)
			{
				from = Subtract(from, to);
				to = Reverse(from);
				return std::min(from, to);
			}

			// Returns the distance from 0 on the number wheel.
			static Modulo Magnitude(Modulo to)
			{
				return Distance(zero, to);
			}

			Modulo()
			: element(zero)
			{
			}

			Modulo(char unsigned const number)
			: element(Normalize(number))
			{
			}
			
			template <char unsigned otherZero>
			Modulo(Modulo<otherZero> other)
			: element(Normalize(other))
			{
			}
			
			virtual ~Modulo()
			{
			}
			
			operator char unsigned() const
			{
				return element;
			}

			// Returns false if invalid.
			bool Set(char unsigned const number)
			{
				if (!IsValid(number))
				{
					return false;
				}
				element = number;
				return true;
			}

		protected:
		
			char unsigned element;
		};
	}
	
	namespace DERM
	{
		namespace Geometry
		{
			struct Cell
			{
				// A namespace that exposes functions for dealing with a hexagon.
				struct Hexagon : Number::Modulo<6>
				{
					// Each cell edge is defined by a rotation.
					// Each cell vertex corresponds to an edge in the other lattice,
					// and is identified by that edge rotation.
					typedef Number::Modulo<6> Rotation;

					static Rotation Opposite()
					{
						static Rotation const opposite(3);
						return opposite;
					}
					
					// Increments edge/vertex rotation to the adjacent vertex/edge.
					static Rotation HalfIncrement(Rotation rotation)
					{
						static Rotation const results[] = {1, 6, 5, 4, 3, 2, 1};
						return results[rotation];
					}

					// Decrements edge/vertex rotation to the adjacent vertex/edge.
					static Rotation HalfDecrement(Rotation rotation)
					{
						static Rotation const results[] = {2, 1, 6, 5, 4, 3, 2};
						return results[rotation];
					}

					static Rotation Reduce(Rotation rotation)
					{
						static Rotation const results[] = {6, 6, 1, 3, 5, 6, 6};
						return results[rotation];
					}
					
					static Rotation Enlarge(Rotation rotation)
					{
						static Rotation const results[] = {6, 2, 3, 3, 3, 4, 6};
						return results[rotation];
					}

					static Rotation Opposite(Rotation rotation)
					{
						return Add(rotation, Opposite());
					}
					
				private:
				
					Hexagon();
				};

				// A namespace that exposes functions for dealing with a hexagon.
				struct Pentagon : Number::Modulo<5>
				{
					// Each cell edge is defined by a rotation.
					// Each cell vertex corresponds to an edge in the other lattice,
					// and is identified by that edge rotation.
					typedef Number::Modulo<5> Rotation;

					// Increments edge/vertex rotation to the adjacent vertex/edge.
					static Rotation HalfIncrement(Rotation rotation)
					{
						static Rotation const results[] = {3, 4, 5, 1, 2, 3};
						return results[rotation];
					}

					// Decrements edge/vertex rotation to the adjacent vertex/edge.
					static Rotation HalfDecrement(Rotation rotation)
					{
						static Rotation const results[] = {3, 2, 1, 5, 4, 3};
						return results[rotation];
					}
					
					// TODO
					static Rotation Reduce(Rotation rotation);
					
					// TODO
					static Rotation Enlarge(Rotation rotation);

				private:
				
					Pentagon();
				};
				
				// A cell offset is a sequence of sub-offsets, any of which may be null,
				// at successive resolutions.
				struct Offset
				{
					// A non-null sub-offset (i.e. offset at a resolution).
					typedef Hexagon::Rotation SubOffset;
				
					Offset()
					: subOffsets()
					{
					}
					
					size_t Resolution() const
					{
						return subOffsets.size();
					}
					
					// Throws if none to pop.
					void Pop()
					{
						subOffsets.pop_back(); // Throws.
					}

					// Push a null sub-offset.
					// Throws if out of memory.
					void Push() 
					{
						subOffsets.push_back(0); // Throws.
					}
					
					// Push a non-null sub-offset.
					// Throws if out of memory.
					void Push(SubOffset const subOffset)
					{
						subOffsets.push_back(subOffset); // Throws.
					}
					
					// If there is a non-null sub-offset at the top
					// (the most-recently pushed), returns true; else, returns false.
					// Throws if empty.
					bool Top() const
					{
						return subOffsets.back(); // Throws.
					}
					
					// If there is a non-null sub-offset at the top 
					// (the most-recently pushed), populates
					// the argument and returns true; otherwise, returns false.
					// Throws if empty.
					bool Top(SubOffset & subOffset) const
					{
						return subOffset.Set(subOffsets.back()); // path.back() throws.
					}
					
					// If there is a non-null sub-offset at the index, returns true;
					// otherwise, returns false.
					// Throws if the index is out of range.
					bool Get(size_t const index) const
					{
						return subOffsets.at(index); // Throws.
					}
					
					// If there is a non-null sub-offset at the index, populates
					// the argument and returns true; otherwise, returns false.
					// Throws if the index is out of range.
					bool Get(size_t const index, SubOffset & subOffset) const
					{
						return subOffset.Set(subOffsets.at(index)); // path.at(...) throws.
					}
					
					// Sets the sub-offset at the index to null.
					// Throws if the index is out of range.
					void Set(size_t const index)
					{
						subOffsets.at(index) = 0; // Throws.
					}
					
					// Sets the sub-offset at the index to the specified non-null
					// sub-offset.
					// Throws if the node is out of range.
					void Set(size_t const index, SubOffset const subOffset)
					{
						subOffsets.at(index) = subOffset; // Throws.
					}
					
				private:

					// A list of nullable sub-offsets at successive resolutions.
					std::vector<char unsigned> subOffsets;
				};

			private:

				// A cached pentagon count for optimization.
				size_t pentagonCount;

				// A cell offset containing the sequence of centroid offsets,
				// starting from root, at successive resolutions.
				Offset offset;

			private:

				bool IsHexagon(size_t const index) const
				{
					return pentagonCount <= index;
				}
				
				// Returns true if it is a centroid offset, or false if it is a centroid.
				bool IsCentroidOffset(size_t const index) const
				{
					return offset.Get(index);
				}

				// Returns true if it is the given centroid offset, or false if not.
				bool IsCentroidOffset(size_t const index, Hexagon::Rotation const centroidOffsetToMatch) const
				{
					Hexagon::Rotation centroidOffset;
					return offset.Get(index, centroidOffset) && centroidOffsetToMatch == centroidOffset;
				}

				// Returns true if it is a centroid offset (and sets 'centroidOffset'
				// to the value), or false if it is a centroid.
				bool GetCentroidOffset(size_t const index, Hexagon::Rotation & centroidOffset) const
				{
					return offset.Get(index, centroidOffset);
				}
				
				void SetCentroidOffset(size_t const index, Hexagon::Rotation const centroidOffset)
				{
					offset.Set(index, centroidOffset);
				}

				void SetCentroid(size_t const index)
				{
					offset.Set(index);
				}

				// Helper function.
				Hexagon::Rotation RotateIn(
					size_t const index,
					Hexagon::Rotation const centroidOffset,
					Hexagon::Rotation const rotation)
				{
					// Moving from a vertex at a resolution greater than 2.
					assert(1 < index);
					assert(Hexagon::Rotation::Magnitude(rotation) == 1);
					assert(IsCentroidOffset(index, centroidOffset));

					if (IsCentroidOffset(index - 1))
					{
						// Resolution 4 equatorial hexagon.
						assert(3 == index);
						assert(IsHexagon(index));
						assert(!IsHexagon(index - 1));
						assert(Hexagon::Opposite() == centroidOffset);
						
						// Half-increment/decrement parent; leaf remains Hexagon::Opposite().
						ToAdjacent(index - 1, rotation);
					} else if (IsHexagon(index - 1))
					{
						// Set leaf to hexagon increment/decrement.
						SetCentroidOffset(index, Hexagon::Add(centroidOffset, rotation));
					} else 
					{
						// Set leaf to pentagon increment/decrement.
						SetCentroidOffset(index, Pentagon::Add(centroidOffset, rotation));
					}
					return Hexagon::Opposite(rotation);
				}
				
				// TODO: VERIFY!
				Hexagon::Rotation ToAdjacent(size_t const index, Hexagon::Rotation edge)
				{
					assert(0 < index);

					{
						Hexagon::Rotation centroidOffset;
						if (GetCentroidOffset(index, centroidOffset)) 
						{
							switch (edge)
							{
							case 2:
								return RotateIn(index, centroidOffset, 1);
							case 4:
								return RotateIn(index, centroidOffset, 5);

							case 1:
							case 5:
								assert(1 < index);

								SetCentroid(index);
								
								if (IsHexagon(index - 1))
								{
									// Move parent from centroid to hexagon half-increment/half-decrement,
									// and return 5 (if 1) or 4 (if 5).
									ToAdjacent(index - 1, Hexagon::HalfAdd(centroidOffset));
									return Pentagon::Decrement(edge); // Even though it's not a pentagon.
								} else
								{
									if (IsCentroidOffset(index - 1))
									{
										// Resolution 4 equatorial hexagon.
										assert(3 == index);
										assert(IsHexagon(index));
										
										ToAdjacent(index - 1, edge);

										// TODO: Finish: should it be 2/4 or 4/2?  Depends on how
										// pentagon orientation is decided.
										
									} else if (!IsHexagon(index))
									{
										// Resolution 3 equatorial pentagon.
										assert(2 == index);
										
										// TODO: Handle res 3 equatorial pentagon.
									}

									// TODO: Move parent from centroid to pentagon half-increment/half-decrement.
								}
								
								// TODO: return proper edge.

							case 6:
								assert(IsHexagon(index));
								assert(2 < index);

								if (!IsHexagon(index - 1))
								{
									// Resolution 4 equatorial hexagon.
									assert(3 == index);
									assert(IsHexagon(index));
									
									// TODO: Denormalize res 4 equatorial hexagon.
								}

								// Get the grandparent edge.
								if (GetCentroidOffset(index - 2, edge))
								{
									if (IsHexagon(index - 2))
									{
										edge = Hexagon::Subtract(centroidOffset, edge);
									} else
									{
										edge = Pentagon::Subtract(centroidOffset, edge);
									}
								} else
								{
									edge = centroidOffset;
								}

								ToAdjacent(index - 2, edge); // Guaranteed to succeed, and sets the edge.
								SetCentroidOffset(index, edge);
								return edge;
							case 3:
								SetCentroid(index);
								if (!IsHexagon(index - 1))
								{
									// TODO: Loop and increase pentagon count until vertex is reached.
								}
								return edge;
							default:
								assert(0);
								break;
							}
						}
					}

					// Moving from centroid.
					assert(!IsCentroidOffset(index));
					
					// Handle vertex centroid cases, which only occur at resolution > 3.
					if (2 < index)
					{
						if (IsHexagon(index))
						{
							if (IsCentroidOffset(index - 1))
							// Move from vertex centroid hexagon.
							{
								// TODO: Move to one of three parents.

								// TODO: Return correct edge
							}
						} else
						{
							// Moving off of a pentagon.
							pentagonCount = index;

							if (IsCentroidOffset(index - 1))
							// Move from vertex centroid pentagon.
							{
								// Resolution 4 equatorial pentagon.
								assert(3 == index);

								// TODO: Move from vertex centroid pentagon according to special logic.
								
								// TODO: Return correct edge
							}
						}
					}

					// Moving from centroid centroid.
					SetCentroidOffset(index, edge);
					return Hexagon::Opposite();
				}

			public:

				// Constructs the root cell.
				Cell()
				: pentagonCount(0), offset()
				{
				}

				// Returns the resolution of the cell.
				//	0 = whole; neither pentagon or hexagon.
				//	1 = 3/4 icosahedron; sign; rotation zero or opposite; root pentagon.
				//	2 = 1/4 icosahedron: centroid child pentagon
				//	3 = dodecahedron: vertex or centroid child pentagon; former has 1 vertex (opposite)
				//	4 = truncated icosahedron: vertex child hexagon or centroid child pentagon
				size_t Resolution() const
				{
					return offset.Resolution();
				}

				// Returns true if this cell is a hexagon.
				bool IsHexagon() const
				{
					size_t const resolution = Resolution();
					return resolution && IsHexagon(resolution - 1);
				}
				
				// Returns true if this cell is offset from a centroid,
				// or false if it is a centroid.
				bool IsCentroidOffset() const
				{
					return offset.Resolution() && offset.Top();
				}

				// Moves to parent.
				// Throws if empty.
				void ToParent()
				{
					offset.Pop(); // Throws.

					// Update pentagon count.
					size_t const resolution = Resolution();
					if (pentagonCount > resolution)
					{
						pentagonCount = resolution;
					}
				}

				// Moves to centroid child.
				// Throws if out of memory.
				void ToChild()
				{
					offset.Push(); // Throws.
				}
				
				// Moves to vertex child.
				// Throws if out of memory.
				// Returns false if there is not a child in the given direction.
				bool ToChild(Hexagon::Rotation const rotation) 
				{
					(void)rotation;
					// ...
					return false;
				}

				// Adds an offset to the index.
				// Left-aligned.
				// Steps in each rotation direction.
				// For resolution 1 and 2: same as moving to adjacent,
				// then pushing/popping to result in the same resolution.
				void Add(Offset const & addend)
				{
					(void)addend;
					// ...
				}
				
				// Subtracts a cell, resulting in an offset.
				// Left-aligned.
				void Subtract(Cell const & subtrahend, Offset & difference) const
				{
					(void)subtrahend;
					(void)difference;
					// ...
				}			

				// If 'edge' gives the name of an edge:
				//	-	Moves to the adjacent cell attached by the edge
				//	-	Sets 'edge' to the name of the same edge in the new cell
				//	-	Returns true.
				// Else, returns false.  The following cells are missing edges:
				//	-	The 0 resolution cell has no edges.
				//	-	A pentagonal cell has no Rotation::zero edge.
				bool ToAdjacent(Hexagon::Rotation & edge)
				{
					// Non-hexagons fall into two camps: pentagons and root,
					// neither of which whill allow 
					if (!IsHexagon() && !Pentagon::IsValid(edge))
					{
						return false;
					}

					// Handle special cases for low resolutions.
					size_t const resolution = Resolution();
					switch (resolution)
					{
					case 0: // Sphere
						return false;
					case 1: // 3/4 icosahedron: move to 1/4 icosahedron centroid at other pole.
						offset.Push();
						break;
					case 2: // 1/4 icosahedron: move to 3/4 icosahedron centroid at other pole.
						offset.Pop();
						break;
					default:
						edge = ToAdjacent(resolution - 1, edge);
						return true;
					}

					// Switch pole; edge stays unchanged.
					{
						Hexagon::Rotation pole;
						assert(IsCentroidOffset(0)); // The first offset must be a vertex.
						GetCentroidOffset(0, pole);
						SetCentroidOffset(0, Hexagon::Opposite(pole));
					}
					
					return true;
				}
			};
		}
	}
}

#endif

int main (int argc, char * const argv[])
{
	(void)argc;
	(void)argv;


	return 0;
}
