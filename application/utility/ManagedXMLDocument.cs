using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Xml.XPath;

namespace ApplicationUtility
{
    public class ManagedXMLDocumentProvider : CSharpXMLDocProvider, IDirectorReferenceCounter
    {
        #region Members 

        private Pyxis.Utilities.TraceTool Trace = new Pyxis.Utilities.TraceTool(true);

        Dictionary<int, XmlDocumentWithCache> m_documents = new Dictionary<int, XmlDocumentWithCache>();

        int m_nextDocIndex = 0;

        #endregion 

        #region RemoveNamesapce logic

        private string RemoveNamespace(string xml)
        {
            var doc = new XmlDocument();
            doc.LoadXml(xml);

            var newDoc = new XmlDocument();

            newDoc.AppendChild(CopyNodeWithoutNamespace(doc.DocumentElement, newDoc));

            return newDoc.OuterXml;
        }

        private XmlElement CopyNodeWithoutNamespace(XmlElement oldElement, XmlDocument newDoc)
        {
            var newElement = newDoc.CreateElement(oldElement.LocalName);

            foreach (XmlAttribute attr in oldElement.Attributes)
            {
                if (attr.Prefix == "xmlns" || attr.Name == "xmlns")
                    continue;

                var newAttr = newDoc.CreateAttribute(attr.LocalName);
                newAttr.Value = attr.Value;

                newElement.Attributes.Append(newAttr);
            }

            if (oldElement.HasChildNodes)
            {
                foreach (XmlNode childNode in oldElement.ChildNodes)
                {
                    switch (childNode.NodeType)
                    {
                        case XmlNodeType.CDATA:
                            {
                                var newChild = newDoc.CreateCDataSection((childNode as XmlCDataSection).InnerText);
                                newElement.AppendChild(newChild);
                            }
                            break;
                        case XmlNodeType.Comment:
                            {
                                var newChild = newDoc.CreateComment((childNode as XmlComment).InnerText);
                                newElement.AppendChild(newChild);
                            }
                            break;
                        case XmlNodeType.Element:
                            {
                                newElement.AppendChild(CopyNodeWithoutNamespace(childNode as XmlElement, newDoc));
                            }
                            break;
                        case XmlNodeType.Text:
                            {
                                var newChild = newDoc.CreateTextNode((childNode as XmlText).InnerText);
                                newElement.AppendChild(newChild);
                            }
                            break;
                        default:
                            Trace.WriteLine("unknown type:" + childNode.NodeType);
                            break;
                    }
                }
            }
            else
            {
                newElement.InnerText = oldElement.InnerText;
            }

            return newElement;
        }

        #endregion

        #region CSharpXMLDocProvider

        public override int createDocument(string xmlString,bool removeNamespace)
        {
            //load the XML document
            XmlDocumentWithCache doc = new XmlDocumentWithCache(removeNamespace ? RemoveNamespace(xmlString) : xmlString);

            int docHandle;

            //lock our manager
            lock (this)
            {
                //create an doc handle
                docHandle = m_nextDocIndex;
                m_nextDocIndex++;

                //store the doc for later use
                m_documents[docHandle] = doc;
            }

            return docHandle;
        }

        public override void destroyDocument(int docHandle)
        {
            lock (this)
            {
                m_documents.Remove(docHandle);
            }
        }

        public override void saveToFile(int docHandle, string path)
        {
            GetDocument(docHandle).Save(path);
        }

        private XmlDocumentWithCache GetDocument(int docHandle)
        {
            lock (this)
            {
                if (m_documents.ContainsKey(docHandle))
                {
                    return m_documents[docHandle];
                }
            }
            return null;
        }

        public override string getInnerXMLString(int docHandle, string xmlPath)
        {
            XmlDocumentWithCache doc = GetDocument(docHandle);

            if (doc == null)
            {
                return "";
            }

            return doc.getInnerXMLString(xmlPath);
        }

        public override void setInnerXMLString(int docHandle, string xmlPath, string innerXml)
        {
            XmlDocumentWithCache doc = GetDocument(docHandle);

            if (doc != null)
            {
                doc.setInnerXMLString(xmlPath, innerXml);
            }
        }
            
        public override string getOuterXMLString(int docHandle, string xmlPath)
        {
            XmlDocumentWithCache doc = GetDocument(docHandle);

            if (doc == null)
            {
                return "";
            }

            return doc.getOuterXMLString(xmlPath);
        }

        public override int getNodesCount(int docHandle, string xmlPath)
        {
            XmlDocumentWithCache doc = GetDocument(docHandle);

            if (doc == null)
            {
                return 0;
            }

            return doc.getNodesCount(xmlPath);
        }

        public override bool hasNode(int docHandle, string xmlPath)
        {
            XmlDocumentWithCache doc = GetDocument(docHandle);

            if (doc == null)
            {
                return false;
            }

            return doc.hasNode(xmlPath);
        }

        public override string getNodeText(int docHandle, string xmlPath)
        {
            XmlDocumentWithCache doc = GetDocument(docHandle);

            if (doc == null)
            {
                return "";
            }

            return doc.getNodeText(xmlPath);
        }

        public override void setNodeText(int docHandle, string xmlPath, string text)
        {
            XmlDocumentWithCache doc = GetDocument(docHandle);

            if (doc != null)
            {
                doc.setNodeText(xmlPath, text);
            }
        }

        public override void addChild(int docHandle, string xmlPath, string xmlChildName)
        {
            XmlDocumentWithCache doc = GetDocument(docHandle);

            if (doc != null)
            {
                doc.addChild(xmlPath, xmlChildName);
            }
        }

        public override void addChildWithInnerText(int docHandle, string xmlPath, string xmlNode, string innerText)
        {
            XmlDocumentWithCache doc = GetDocument(docHandle);

            if (doc != null)
            {
                doc.addChildWithInnderText(xmlPath, xmlNode, innerText);
            }
        }

        public override void removeNode(int docHandle, string xmlPath)
        {
            XmlDocumentWithCache doc = GetDocument(docHandle);

            if (doc != null)
            {
                doc.removeNode(xmlPath);
            }
        }

        public override bool hasAttribute(int docHandle, string xmlPath, string attributeName)
        {
            XmlDocumentWithCache doc = GetDocument(docHandle);

            if (doc != null)
            {
                return doc.hasAttribute(xmlPath, attributeName);
            }
            return false;
        }

        public override void addAttribute(int docHandle, string xmlPath, string attributeName, string attributeValue)
        {
            XmlDocumentWithCache doc = GetDocument(docHandle);

            if (doc != null)
            {
                doc.addAttribute(xmlPath, attributeName, attributeValue);
            }
        }

        public override void removeAttribute(int docHandle, string xmlPath, string attributeName)
        {
            XmlDocumentWithCache doc = GetDocument(docHandle);

            if (doc != null)
            {
                doc.removeAttribute(xmlPath, attributeName);
            }
        }

        public override string getAttributeValue(int docHandle, string xmlPath, string attributeName)
        {
            XmlDocumentWithCache doc = GetDocument(docHandle);

            if (doc != null)
            {
                return doc.getAttributeValue(xmlPath, attributeName);
            }
            return "";
        }

        public override void setAttributeValue(int docHandle, string xmlPath, string attributeName, string attributeValue)
        {
            XmlDocumentWithCache doc = GetDocument(docHandle);

            if (doc != null)
            {
                doc.setAttributeValue(xmlPath, attributeName, attributeValue);
            }
        }

        public override void addNamespace(int docHandle, string prefix, string uri)
        {
            XmlDocumentWithCache doc = GetDocument(docHandle);

            if (doc != null)
            {
                doc.AddNamespace(prefix,uri);
            }
        }

        #endregion

        #region PYXObject Lifetime Management

        #region IDirectorReferenceCounter Members

        public void setSwigCMemOwn(bool value)
        {
            swigCMemOwn = value;
        }

        public int doAddRef()
        {
            return base.addRef();
        }

        public int doRelease()
        {
            return base.release();
        }

        #endregion

        #region PYXObject

        /// <summary>
        /// Override the reference-counting addRef.  This is not called 
        /// directly!
        /// </summary>
        /// <returns>Current reference count (after increment).</returns>
        public override int addRef()
        {
            if (getCPtr(this).Handle == IntPtr.Zero)
            {
                return 1;
            }
            return ReferenceManager.addRef(this);
        }

        /// <summary>
        /// Override the reference-counting release.  This is not called 
        /// directly!
        /// </summary>
        /// <returns>Current reference count (after decrement).</returns>
        public override int release()
        {
            if (getCPtr(this).Handle == IntPtr.Zero)
            {
                return 1;
            }
            return ReferenceManager.release(this);
        }

        #endregion
        /// <summary>
        /// Default constructor.
        /// </summary>
        public ManagedXMLDocumentProvider()
        {
        }

        #endregion PYXObject Lifetime Management
    }

    internal class XmlDocumentWithCache : XmlDocument
    {
        #region Members

        protected XmlNode lastSingleNode;
        protected string lastSingleNodeXmlPath;

        protected XmlNodeList lastNodeList;
        protected string lastNodeListXmlPath;

        protected XmlNamespaceManager activeNamespaces;

        #endregion

        #region Constructor

        public XmlDocumentWithCache(string xmlString)
        {
            try
            {
                if (xmlString != "")
                {
                    LoadXml(xmlString);
                }

                activeNamespaces = new XmlNamespaceManager(this.NameTable);
            }
            catch (Exception e)
            {
                Trace.info("Failed to parse XML document: " + e.Message);
            }
        }

        #endregion

        #region Fetching nodes

        private XmlNodeList FindAllNodes(string xmlPath)
        {
            lock (this)
            {
                if (xmlPath != lastNodeListXmlPath)
                {
                    lastNodeListXmlPath = xmlPath;
                    lastNodeList = null;

                    try
                    {
                        lastNodeList = SelectNodes(lastNodeListXmlPath, activeNamespaces);
                    }
                    catch (Exception)
                    {
                    }
                }

                return lastNodeList;
            }
        }

        private XmlNode FindPath(string xmlPath)
        {
            lock (this)
            {
                if (xmlPath != lastSingleNodeXmlPath)
                {
                    lastSingleNodeXmlPath = xmlPath;
                    lastSingleNode = null;

                    try
                    {
                        lastSingleNode = SelectSingleNode(lastSingleNodeXmlPath, activeNamespaces);
                    }
                    catch (Exception)
                    {
                    }
                }

                return lastSingleNode;
            }         
        }

        private void ClearCache()
        {
            lastSingleNodeXmlPath = "";
            lastSingleNode = null;

            lastNodeListXmlPath = "";
            lastNodeList = null;
        }

        #endregion 

        #region Document modifications

        public int getNodesCount(string xmlPath)
        {
            lock (this)
            {
                XmlNodeList nodes = FindAllNodes(xmlPath);

                if (nodes == null)
                {
                    return 0;
                }

                return nodes.Count;
            }
        }

        public string getInnerXMLString(string xmlPath)
        {
            lock (this)
            {
                XmlNode node = FindPath(xmlPath);

                if (node == null)
                {
                    return "";
                }

                return node.InnerXml;
            }
        }

        public void setInnerXMLString(string xmlPath, string innerXml)
        {
            lock (this)
            {
                XmlNode node = FindPath(xmlPath);

                if (node != null)
                {
                    node.InnerXml = innerXml;
                    ClearCache();
                }
            }
        }

        public string getOuterXMLString(string xmlPath)
        {
            lock (this)
            {
                XmlNode node = FindPath(xmlPath);

                if (node == null)
                {
                    return "";
                }

                return node.OuterXml;
            }
        }

        public bool hasNode(string xmlPath)
        {
            lock (this)
            {
                XmlNode node = FindPath(xmlPath);

                return node != null;
            }
        }

        public string getNodeText(string xmlPath)
        {
            lock (this)
            {
                XmlNode node = FindPath(xmlPath);

                if (node != null)
                {
                    return node.InnerText;
                }
                return "";
            }
        }

        public void setNodeText(string xmlPath, string text)
        {
            lock (this)
            {
                XmlNode node = FindPath(xmlPath);

                if (node != null)
                {
                    node.InnerText = text;
                }
                ClearCache();
            }
        }        

        public XmlNode addChild(string xmlPath, string xmlChildName)
        {
            lock (this)
            {
                XmlNode node = FindPath(xmlPath);

                if (node != null)
                {
                    try
                    {
                        XmlNode newNode = null;
                        if (xmlChildName.Contains(":"))
                        {
                            var nameParts = xmlChildName.Split(new[] {':'}, 2);
                            var prefix = nameParts[0];
                            var localName = nameParts[1];
                            var namespaceUri = activeNamespaces.LookupNamespace(nameParts[0]);
                            if (DocumentElement.NamespaceURI == namespaceUri)
                            {
                                prefix = "";
                            }
                            newNode = CreateElement(prefix, localName, namespaceUri);
                        }
                        else
                        {
                            newNode = CreateElement(xmlChildName);
                        }

                        node.AppendChild(newNode);
                        ClearCache();
                        return newNode;
                    }
                    catch (Exception e)
                    {
                        System.Windows.Forms.MessageBox.Show("Failed to add child : " + e.Message);
                        return null;
                    }
                }
                return null;
            }
        }

        public void addChildWithInnderText(string xmlPath, string xmlNode, string innerText)
        {
            lock (this)
            {
                var node = addChild(xmlPath, xmlNode);
                if (node != null)
                {
                    node.InnerText = innerText;
                }
            }
        }

        public void removeNode(string xmlPath)
        {
            lock (this)
            {
                XmlNode node = FindPath(xmlPath);

                if (node != null)
                {
                    node.ParentNode.RemoveChild(node);
                    ClearCache();
                }
            }
        }

        public bool hasAttribute(string xmlPath, string attributeName)
        {
            lock (this)
            {
                XmlNode node = FindPath(xmlPath);

                if (node != null)
                {
                    return node.Attributes[attributeName] != null;
                }
                return false;
            }
        }

        public void addAttribute(string xmlPath, string attributeName, string attributeValue)
        {
            lock (this)
            {
                XmlNode node = FindPath(xmlPath);

                if (node != null)
                {

                    try
                    {
                        if (node.Attributes[attributeName] == null)
                        {
                            XmlAttribute newAttribute = CreateNode(XmlNodeType.Attribute, attributeName, "") as XmlAttribute;
                            newAttribute.Value = attributeValue;
                            node.Attributes.Append(newAttribute);
                            ClearCache();
                        }
                    }
                    catch (Exception)
                    {             
                    }
                }
            }
        }

        public void removeAttribute(string xmlPath, string attributeName)
        {
            lock (this)
            {
                XmlNode node = FindPath(xmlPath);

                if (node != null)
                {
                    XmlAttribute attribute = node.Attributes[attributeName];
                    if (attribute != null)
                    {
                        node.Attributes.Remove(attribute);
                        ClearCache();
                    }
                }
            }
        }

        public string getAttributeValue(string xmlPath, string attributeName)
        {
            XmlNode node = FindPath(xmlPath);

            if (node != null)
            {
                XmlAttribute attribute = node.Attributes[attributeName];
                if (attribute != null)
                {
                    return attribute.Value;
                }
            }
            return "";
        }

        public void setAttributeValue(string xmlPath, string attributeName, string attributeValue)
        {
            lock (this)
            {
                XmlNode node = FindPath(xmlPath);

                if (node != null)
                {
                    XmlAttribute attribute = node.Attributes[attributeName];
                    if (attribute != null)
                    {
                        attribute.Value = attributeValue;
                        ClearCache();
                    }
                }
            }
        }
        
        #endregion

        #region Namespace modifications

        public void AddNamespace(string name,string uri)
        {
            if (String.IsNullOrEmpty(DocumentElement.GetPrefixOfNamespace(uri)) && DocumentElement.NamespaceURI != uri)
            {
                //do not override existing namespace declaration. but add if needed
                DocumentElement.SetAttribute("xmlns:" + name, uri);    
            }
            activeNamespaces.AddNamespace(name,uri);
        }

        #endregion

        
    }
}
