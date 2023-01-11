using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Pyxis.Contract.Workspaces
{
    /// <summary>
    /// Reference is a way to point to a dataset (and even a field inside a dataset).
    /// 
    /// The format of a Refernece is "[field] @ workspace [/ name [?domain=value&domain=value] ]" (without the spaces)
    /// The field and name and domains values are optional
    /// 
    /// TODO: we need to add the support for difference origins (cluster)
    /// </summary>
    public class Reference
    {
        public string Field { get; set; }
        public string Workspace { get; set; }
        public string Name { get; set; }
        public Dictionary<string,string> Domains { get; set; }

        public static string SanatizeReference(string refernce)
        {
            if (refernce != null && refernce.StartsWith("[") && refernce.EndsWith("]"))
            {
                return refernce.Substring(1, refernce.Length - 2);
            }
            return refernce;
        }

        public static Reference FromParts(string workspace, string name, Dictionary<string, string> domains = null, string field = null)
        {
            return new Reference(workspace, name, domains, field);
        }

        public static string EncodeFromParts(string workspace, string name, Dictionary<string, string> domains = null, string field = null)
        {
            return FromParts(workspace, name, domains, field).ToString();
        }

        private Reference(string workspace, string name, Dictionary<string, string> domains = null, string field = null)
        {
            Workspace = workspace;
            Name = name;
            Domains = domains;
            Field = field;
        }

        public Reference(string value)
        {
            value = SanatizeReference(value);

            var index = value.IndexOf('/');
            if (index >= 0)
            {
                Workspace = value.Substring(0, index);
                Name = value.Substring(index + 1);

                var fieldIndex = Workspace.IndexOf('@');
                
                if (fieldIndex >= 0)
                {
                    Field = Workspace.Substring(0, fieldIndex);
                    Workspace = Workspace.Substring(fieldIndex + 1);
                }

                var queryIndex = Name.IndexOf('?');
                if (queryIndex >= 0)
                {
                    var query = Name.Substring(queryIndex + 1);
                    Name = Name.Substring(0, queryIndex);

                    var collection = HttpUtility.ParseQueryString(query);

                    Domains = collection.AllKeys.ToDictionary(k => k, k => collection[k]);
                }
            }
        }

        public override string ToString()
        {
            var str = Workspace;
            if (!String.IsNullOrEmpty(Name))
            {
                str += '/' + Name;
            }

            if (Domains != null && Domains.Count > 0)
            {
                str += '?' +
                       string.Join("&", Domains.Keys.Select(key => key + "=" + HttpUtility.UrlEncode(Domains[key])));
            }

            if (!String.IsNullOrEmpty(Field))
            {
                str = Field + "@" + str;
            }

            return str;
        }
    }
}
