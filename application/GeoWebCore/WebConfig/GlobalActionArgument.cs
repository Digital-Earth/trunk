namespace GeoWebCore.WebConfig
{
    /// <summary>
    /// Global names for http request action argument been used by the application
    /// </summary>
    internal static class GlobalActionArgument
    {
        /// <summary>
        /// Used to store the user id authenticated by the request
        /// </summary>
        public static readonly string UserId = "UserId";
        
        /// <summary>
        /// Used to store the gallery id authenticated by the request
        /// </summary>
        public static readonly string GalleryId = "GalleryId";

        /// <summary>
        /// The user token that was used by the request
        /// </summary>
        public static readonly string UserToken = "UserToken";
    }
}
