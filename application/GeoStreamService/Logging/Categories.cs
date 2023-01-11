using Pyxis.Utilities.Logging;

namespace GeoStreamService.Logging
{
    public static class Categories
    {
        public static LogCategory Jobs = new LogCategory("Gwss.Jobs");
        public static LogCategory Crashes = new LogCategory("Gwss.Crashes");
        public static LogCategory GalleryTest = new LogCategory("Gwss.GalleryTest");
    }
}