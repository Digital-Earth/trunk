using System.Drawing;
using ApplicationUtility;
using Pyxis.UI.Properties;

namespace Pyxis.UI
{
    /// <summary>
    /// Utility for creating instances of common System.Drawing.Bitmap for use as icons. 
    /// </summary>
    public static class Icons
    {
        /// <summary>
        /// Create a circle.
        /// </summary>
        public static Bitmap Circle
        {
            get
            {
                return ManagedBitmapServer.Instance.LoadBitmapFromDefinition(Resources.IconCircle);
            }
        }

        /// <summary>
        /// Create a box.
        /// </summary>
        public static Bitmap Box
        {
            get
            {
                return ManagedBitmapServer.Instance.LoadBitmapFromDefinition(Resources.IconBox);
            }
        }

        /// <summary>
        /// Create a star.
        /// </summary>
        public static Bitmap Star
        {
            get
            {
                return ManagedBitmapServer.Instance.LoadBitmapFromDefinition(Resources.IconStar);
            }
        }

        /// <summary>
        /// Create a Hexagon.
        /// </summary>
        public static Bitmap Hexagon
        {
            get
            {
                return ManagedBitmapServer.Instance.LoadBitmapFromDefinition(Resources.IconHexagon);
            }
        }

        /// <summary>
        /// Create a place.
        /// </summary>
        public static Bitmap Place
        {
            get
            {
                return ManagedBitmapServer.Instance.LoadBitmapFromDefinition(Resources.IconPlace);
            }
        }

        /// <summary>
        /// Create a triangle.
        /// </summary>
        public static Bitmap Triangle
        {
            get
            {
                return ManagedBitmapServer.Instance.LoadBitmapFromDefinition(Resources.IconTriangle);
            }
        }
    }
}
