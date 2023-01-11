using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.Serialization;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace Pyxis.Contract.Workspaces
{
    /// <summary>
    /// Utilitly class to parse Workspace Json files.
    /// 
    /// This class help resolving 'reference' to external files inside the workspace json file format.
    /// </summary>
    public static class WorkspaceParser
    {
        public static Workspace ReadFile(string file)
        {
            var directory = Path.GetDirectoryName(file);

            var workspace = new Workspace()
            {
                Endpoints = new Dictionary<string, Endpoint>(),
                Imports = new Dictionary<string, IImport>(),
                Globes = new Dictionary<string, GlobeTemplate>()
            };

            dynamic json;
            try
            {
                json = JsonConvert.DeserializeObject(File.ReadAllText(file));
            }
            catch (Exception ex)
            {
                throw new ParsingException(string.Format("Failed to parse workspace file: {0}", file), ex);
            }

            try
            {
                if (json.endpoints != null)
                {
                    foreach (var property in json.endpoints)
                    {
                        workspace.Endpoints[property.Name] = new Endpoint
                        {
                            Uri = property.Value
                        };
                    }
                }
            }
            catch (Exception ex)
            {
                throw new ParsingException(string.Format("Failed to parse endpoints of workspace file: {0}", file), ex);
            }

            try
            {
                if (json.imports != null)
                {
                    foreach (var property in json.imports)
                    {
                        if (property.Value.Type == JTokenType.String)
                        {
                            workspace.Imports[property.Name] =
                                ParseImport(
                                    JsonConvert.DeserializeObject(
                                        File.ReadAllText(Path.Combine(directory, property.Value.ToString()))));
                        }
                        else
                        {
                            workspace.Imports[property.Name] = ParseImport(property.Value);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                throw new ParsingException(string.Format("Failed to parse imports of workspace file: {0}", file), ex);
            }

            try
            {
                if (json.globes != null)
                {
                    foreach (var property in json.globes)
                    {
                        if (property.Value.Type == JTokenType.String)
                        {
                            workspace.Globes[property.Name] =
                                JsonConvert.DeserializeObject<GlobeTemplate>(
                                    File.ReadAllText(Path.Combine(directory, property.Value.ToString())));
                        }
                        else
                        {
                            workspace.Globes[property.Name] = property.Value.ToObject<GlobeTemplate>();
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                throw new ParsingException(string.Format("Failed to parse globes of workspace file: {0}", file), ex);
            }

            try
            {
                if (json.owner != null)
                {
                    workspace.Owner = json.owner.ToObject<OnwerDetails>();
                }
            }
            catch (Exception ex)
            {
                throw new ParsingException(string.Format("Failed parse onwer section of workspace file: {0}", file), ex);
            }

            try
            {
                if (json.permissions != null)
                {
                    workspace.Permissions = json.permissions.ToObject<Dictionary<string, List<string>>>();
                }
            }
            catch (Exception ex)
            {
                throw new ParsingException(
                    string.Format("Failed to parse permission section of workspace file: {0}", file), ex);
            }

            return workspace;
        }

        public static Workspace CreateEmptyWorkspace(string workspaceFilename)
        {
            var workspace = Workspace.CreateEmpty();
            File.WriteAllText(workspaceFilename, JsonConvert.SerializeObject(workspace, Formatting.Indented));
            return workspace;
        }

        public static IImport ParseImport(dynamic obj)
        {
            if (obj["Type"] == "DataSetTemplate")
            {
                return obj.ToObject<ImportTemplate>();
            }
            return obj.ToObject<ImportDataSet>();
        }

        public static void UpdateOrInsertEndpoint(string workspaceFile, string name, Endpoint endpoint, string importFilename = null)
        {
            UpdateWorkspace(workspaceFile, json =>
            {
                if (json.endpoints == null)
                {
                    var imports = new Dictionary<string, object>();
                    imports[name] = endpoint.Uri;
                    json.endpoints = imports;
                }
                else
                {
                    json.endpoints[name] = endpoint.Uri;
                }
            });
        }

        public static void UpdateOrInsertImport(string workspaceFile, string name, IImport import, string importFilename = null)
        {
            UpdateWorkspace(workspaceFile, json =>
            {
                if (json.imports == null)
                {
                    var imports = new Dictionary<string, object>();
                    imports[name] = SeralizeObjectInlineOrToFile(import,workspaceFile, importFilename);
                    json.imports = imports;
                }
                else
                {
                    json.imports[name] = SeralizeObjectInlineOrToFile(import,workspaceFile, importFilename);
                }
            });
        }

        public static void UpdateOrInsertGlobe(string workspaceFile, string name, GlobeTemplate globeTemplate, string globeFilename = null)
        {
            UpdateWorkspace(workspaceFile, json =>
            {
                if (json.globes == null)
                {
                    var globes = new Dictionary<string, object>();
                    globes[name] = SeralizeObjectInlineOrToFile(globeTemplate, workspaceFile, globeFilename);
                    json.globes = globes;
                }
                else
                {
                    json.globes[name] = SeralizeObjectInlineOrToFile(globeTemplate, workspaceFile, globeFilename);
                }
            });
        }

        public static void RemoveImport(string workspaceFile, string name)
        {
            UpdateWorkspace(workspaceFile, json =>
            {
                if (json.imports != null)
                {
                    (json.imports as JObject).Remove(name);
                }
            });
        }

        public static void RemoveGlobe(string workspaceFile, string name)
        {
            UpdateWorkspace(workspaceFile, json =>
            {
                if (json.globes != null)
                {
                    (json.globes as JObject).Remove(name);
                }
            });
        }

        public static void RemoveEndpoint(string workspaceFile, string name)
        {
            UpdateWorkspace(workspaceFile, json =>
            {
                if (json.endpoints != null)
                {
                    (json.endpoints as JObject).Remove(name);
                }
            });
        }

        private static JToken SeralizeObjectInlineOrToFile(object obj, string workspaceFile, string filename = null)
        {
            
            if (string.IsNullOrEmpty(filename))
            {
                return JObject.FromObject(obj);
            }

            var directory = Path.GetDirectoryName(workspaceFile);
            File.WriteAllText(Path.Combine(directory, filename) , JsonConvert.SerializeObject(obj, Formatting.Indented));
            return filename;
        }

        private static void UpdateWorkspace(string workspaceFilename, Action<dynamic> modifyAction)
        {
            dynamic json = JsonConvert.DeserializeObject(File.ReadAllText(workspaceFilename));

            modifyAction(json);

            File.WriteAllText(workspaceFilename, JsonConvert.SerializeObject(json, Formatting.Indented));
        }     
    }

    [Serializable]
    public class ParsingException : Exception
    {
        public ParsingException()
        {
        }

        public ParsingException(string message) : base(message)
        {
        }

        public ParsingException(string message, Exception inner) : base(message, inner)
        {
        }

        protected ParsingException(
            SerializationInfo info,
            StreamingContext context) : base(info, context)
        {
        }
    }
}