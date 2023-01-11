using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Contract.Converters
{
    /// <summary>
    /// Convert System.Drawing.Image or System.Drawing.Icon into data url.
    /// </summary>
    public static class DataUrlConverter
    {
        /// <summary>
        /// Convert System.Drawing.Image into data url using PNG format.
        /// </summary>
        /// <param name="image">Image to convert into Data url.</param>
        /// <returns>string represent a data url.</returns>
        public static string ToDataUrl(this Image image)
        {
            using (var stream = new MemoryStream())
            {
                image.Save(stream, ImageFormat.Png);
                return string.Format("data:image/png;base64,{0}", Convert.ToBase64String(stream.GetBuffer()));
            }
        }

        /// <summary>
        /// Convert System.Drawing.Icon into data url using PNG format.
        /// </summary>
        /// <param name="icon">Icon to convert into Data url.</param>
        /// <returns>string represent a data url.</returns>
        public static string ToDataUrl(this Icon icon)
        {
            using (var stream = new MemoryStream())
            {
                icon.Save(stream);
                return string.Format("data:image/x-icon;base64,{0}", Convert.ToBase64String(stream.GetBuffer()));
            }
        }
    }
}
