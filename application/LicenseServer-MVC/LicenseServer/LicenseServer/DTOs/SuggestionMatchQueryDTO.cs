using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace LicenseServer.DTOs
{
    public class SuggestionMatchQueryDTO
    {
        public string Text { get; set; }
        public Pyxis.Contract.Publishing.ResourceType[] Types;
    }
}