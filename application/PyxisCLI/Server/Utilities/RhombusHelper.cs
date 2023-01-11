using System;
using System.Linq;

namespace PyxisCLI.Server.Utilities
{
    /// <summary>
    /// helper functions to deal correctly with Rhombus indices.
    /// </summary>
    internal static class RhombusHelper
    {
        /// <summary>
        /// Check if this a top level rhombus. "0"..."9"
        /// </summary>
        /// <param name="key">rhombus key</param>
        /// <returns>true if top level rhombus</returns>
        public static bool IsTopRhombus(string key)
        {
            return key.Length == 1;
        }

        /// <summary>
        /// Create the right PYXRhombus object for a given rhombus key. please note, PYXRhombus are using old style rhombus numbering
        /// </summary>
        /// <param name="key">Rhombus key</param>
        /// <returns>PYXRhombus object</returns>
        public static PYXRhombus GetRhombusFromKey(string key)
        {
            var fixedKey = FixRhombusKey(key);
            var root = int.Parse(fixedKey.Substring(0, 2));
            var result = new PYXRhombus(root);

            foreach (var c in key.Skip(2))
            {
                result = result.getSubRhombus(c - '0');
            }

            return result;
        }

        /// <summary>
        /// Fix a rhombus key from new indexing method to old indexing method.
        /// </summary>
        /// <param name="key">new rhombus index</param>
        /// <returns>old rhombus index.</returns>
        public static string FixRhombusKey(string key)
        {
            //convert first two chars from hierarchy index([0..9][0..8]) to PYXRhombus root([0..89])
            int rootRhombus = key[0] - '0';
            int childRhombus = key[1] - '0';
            int oldRhombusIndex = rootRhombus * 9 + childRhombus;
            return String.Format("{0:00}{1}", oldRhombusIndex, key.Substring(2));
        }

        /// <summary>
        /// Get lower size of a rhombus (1 resolution lower)
        /// </summary>
        /// <param name="size">valid rhombus size</param>
        /// <returns>rhombus size of a lower resolution</returns>
        public static int GetLowerSize(int size)
        {
            switch (size)
            {
                case 10:
                    return 4;
                case 28:
                    return 10;
                case 82:
                    return 28;
                case 244:
                    return 82;
            }
            throw new Exception("size is not valid");
        }

        /// <summary>
        /// Return true if this is a valid rhombus size
        /// </summary>
        /// <param name="size">size of a rhombus</param>
        /// <returns>true if size is valid</returns>
        public static bool IsValidSize(int size)
        {
            switch (size)
            {
                case 10:
                case 28:
                case 82:
                case 244:
                    return true;

                default:
                    return false;
            }
        }

        /// <summary>
        /// Convert rhombus size to matching Pyxis tiles needed to be loaded
        /// </summary>
        /// <param name="size">rhombus size</param>
        /// <returns>Pyxis tile depth</returns>
        public static int SizeToResolution(int size)
        {
            switch (size)
            {
                case 10:
                    return 4;

                case 28:
                    return 6;

                case 82:
                    return 8;

                case 244:
                    return 10;

                default:
                    throw new Exception("size is not valid");
            }
        }

        /// <summary>
        /// Return cell offset of a child rhombus inside the parent rhombus
        /// </summary>
        /// <param name="index">child offset</param>
        /// <param name="parentRhombusSize">parent rhombus size</param>
        /// <returns>cell offset</returns>
        public static int ChildRhombusOffset(int index, int parentRhombusSize)
        {
            var childRhombusSize = GetLowerSize(parentRhombusSize);

            //Make -1 because our child rhombuses is overlapping
            var left = (index % 3) * (childRhombusSize - 1);
            var top = (index / 3) * (childRhombusSize - 1) * parentRhombusSize;
            return left + top;
        }

        /// <summary>
        /// Check if a all tiles needed to fill a rhobmus are already cached.
        /// </summary>
        /// <param name="key">Rhombus key</param>
        /// <param name="size">Rhombus size</param>
        /// <param name="coverage">ICoverage to check</param>
        /// <returns>true if all tiles found in cache</returns>
        public static bool WillLoadFast(string key, int size, ICoverage_SPtr coverage)
        {
            if (!IsTopRhombus(key))
            {
                var rhombus = GetRhombusFromKey(key);
                return PYXRhombusUtils.willLoadFast(rhombus, SizeToResolution(size), coverage);
            }
            else
            {
                var subRhombusResolution = SizeToResolution(size) - 2;

                var rhombusIndex = int.Parse(key);

                for (var childRhombus = 0; childRhombus < 9; childRhombus++)
                {
                    var rhombus = new PYXRhombus(rhombusIndex * 9 + childRhombus);
                    if (!PYXRhombusUtils.willLoadFast(rhombus, subRhombusResolution, coverage))
                    {
                        return false;
                    }
                }
                return true;
            }
        }

        public static void RhombusOffsetToUv(int offset, int size, out int u, out int v)
        {
            u = offset%size;
            v = offset/size;
        }

        public static int RhombusUvToOffset(int size, int u, int v)
        {
            return v*size + u;
        }

        /// <summary>
        /// Generate unit sphere cell centers for a given rhombus
        /// </summary>
        /// <param name="key">rhombus key</param>
        /// <param name="size">rhombus size</param>
        /// <returns>list of [x,y,z,x,y,z,...] for all cells in rhombus</returns>
        public static double[] GetRhombusVerticies(string key, int size)
        {
            var vertices = new double[size * size * 3];
            if (IsTopRhombus(key))
            {
                var subRhombusSize = GetLowerSize(size);
                var rhombusIndex = int.Parse(key);

                for (var childRhombus = 0; childRhombus < 9; childRhombus++)
                {
                    var rhombus = new PYXRhombus(rhombusIndex * 9 + childRhombus);
                    var cursor = new PYXRhombusCursor(rhombus, SizeToResolution(size) - 2);

                    var pi = 0;
                    var parentOffset = ChildRhombusOffset(childRhombus, size);

                    while (!cursor.end())
                    {
                        int u,v;

                        RhombusOffsetToUv(pi, subRhombusSize, out u, out v);
                        var index = RhombusUvToOffset(size, u, v) + parentOffset;

                        var p = PointLocation.fromPYXIndex(cursor.getIndex());
                        var xyz = p.asXYZ();

                        vertices[index * 3 + 0] = xyz.x();
                        vertices[index * 3 + 1] = xyz.y();
                        vertices[index * 3 + 2] = xyz.z();

                        cursor.next();
                        pi++;
                    }
                }
            }
            else
            {
                var rhombus = GetRhombusFromKey(key);
                var cursor = new PYXRhombusCursor(rhombus, SizeToResolution(size));

                var pi = 0;
                while (!cursor.end())
                {
                    var p = PointLocation.fromPYXIndex(cursor.getIndex());
                    var xyz = p.asXYZ();

                    vertices[pi * 3 + 0] = xyz.x();
                    vertices[pi * 3 + 1] = xyz.y();
                    vertices[pi * 3 + 2] = xyz.z();

                    cursor.next();
                    pi++;
                }
            }

            return vertices;
        }
    }
}