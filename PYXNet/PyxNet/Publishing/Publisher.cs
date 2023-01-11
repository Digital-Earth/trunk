/******************************************************************************
Publisher.cs

begin      : 18/10/2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace PyxNet.Publishing
{
    /// <summary>
    /// A generic publisher.  A publisher attaches to a stack, and publishes
    /// a collection of items to that stack.  It responds to queries on the 
    /// stack and handle sthem appropriately (sending back QueryResults.)
    /// </summary>
    /// <remarks>
    /// TODO: Add database?  Maybe not!
    /// TODO: We don't allow items to be "unpublished".
    /// TODO: We don't remove items from the QHT when the publisher stops.
    /// TODO: Allow the data to live "offline", instead of keeping it all in 
    /// memory all the time.
    /// </remarks>
    public class Publisher
    {
        /// <summary>
        /// Interface for a published item.
        /// </summary>
        public interface IPublishedItemInfo
        {
            /// <summary>
            /// Gets the keywords for this published item.  This is the
            /// set of terms that the item will be indexed on in the 
            /// query hash table.
            /// </summary>
            /// <value>The keywords.</value>
            IEnumerable<string> Keywords
            { get;}

            /// <summary>
            /// Does this match the specified query?
            /// </summary>
            /// <param name="query">The query.</param>
            /// <param name="stack">The stack.</param>
            /// <returns>
            /// True if this matches the specified query.
            /// </returns>
            QueryResult Matches(Query query, Stack stack);
        }

        /// <summary>
        /// Interface for a published item with changing keywords.
        /// </summary>
        public interface IDynamicPublishedItem : IPublishedItemInfo
        {
            /// <summary>
            /// Event to raise when the keywords of the item need changing
            /// </summary>
            event EventHandler KeywordsChanged;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="Publisher"/> class.
        /// </summary>
        /// <param name="stack">
        /// The stack to which the publisher is connected.
        /// </param>
        public Publisher(Stack stack)
        {
            m_stack = stack;
            Start();
        }

        /// <summary>
        /// Starts this instance.
        /// </summary>
        public void Start()
        {
            m_stack.OnQueryHit += HandleQueryHit;
        }

        /// <summary>
        /// Stops this instance.
        /// </summary>
        public void Stop()
        {
            this.Stack.OnQueryHit -= HandleQueryHit;
        }

        /// <summary>
        /// Unpublish a published item.
        /// The query hash table will not be updated, potentially
        /// causing higher false positives on query propagation.
        /// Call UnpublishItemAndRebuildQHT to also rebuild the hash 
        /// table.
        /// </summary>
        /// <param name="item"></param>
        public void UnpublishItem(IPublishedItemInfo item)
        {
            PublishedItems.Remove(item);
        }

        /// <summary>
        /// Unpublish a published item and rebuild the query hash table.
        /// This method is slower than UnpublishItem but the query hash table
        /// will accurately reflect the published items on completion.
        /// </summary>
        /// <param name="item"></param>
        public void UnpublishItemAndRebuildQHT(IPublishedItemInfo item)
        {
            UnpublishItem(item);
            RebuildQueryHashTable();
        }

        /// <summary>
        /// Clear the items published by the publisher
        /// </summary>
        public void ClearItems()
        {
            foreach (IDynamicPublishedItem dynamicPublishedItem in PublishedItems.Where(x => x is IDynamicPublishedItem))
            {
                dynamicPublishedItem.KeywordsChanged -= OnKeywordsChanged;
            }
            PublishedItems.Clear();
        }

        private Pyxis.Utilities.DynamicList<IPublishedItemInfo> m_publishedItems =
            new Pyxis.Utilities.DynamicList<IPublishedItemInfo>();

        /// <summary>
        /// Gets the published items.
        /// </summary>
        /// <value>The published items.</value>
        private Pyxis.Utilities.DynamicList<IPublishedItemInfo> PublishedItems
        {
            get { return m_publishedItems; }
        }
        
        /// <summary>
        /// Publishes an item.
        /// </summary>
        /// <param name="item">The item.</param>
        public void PublishItem(IPublishedItemInfo item)
        {
            this.PublishedItems.Add(item);
            if (item is IDynamicPublishedItem)
            {
                var dynamicPublishedItem = item as IDynamicPublishedItem;
                dynamicPublishedItem.KeywordsChanged += OnKeywordsChanged;
            }
            this.AddToQueryHashTable(item);
        }

        public IList<IPublishedItemInfo> FindPublishedItemsByType<T>() where T : IPublishedItemInfo
        {
            return PublishedItems.Where(x => x.GetType() == typeof(T)).ToList();
        }

        private readonly Stack m_stack;

        /// <summary>
        /// Gets the stack.
        /// </summary>
        /// <value>The stack.</value>
        private Stack Stack
        {
          get { return m_stack; }
        }

        /// <summary>
        /// Adds the given item to the Stack's query hash table.
        /// </summary>
        /// <param name="item">The item.</param>
        private void AddToQueryHashTable(IPublishedItemInfo item, QueryHashTable queryHashTable)
        {
            try
            {
                queryHashTable.BeginUpdate();

                foreach (string keyword in item.Keywords)
                {
                    queryHashTable.Add(keyword);

                    this.Stack.Tracer.DebugWriteLine(
                        "Publisher adding {0} to local QHT of stack '{1}'.", keyword, this.Stack.NodeInfo == null ? "" : this.Stack.NodeInfo.FriendlyName);
                }
            }
            finally
            {
                queryHashTable.EndUpdate();
            }
        }

        private void AddToQueryHashTable(IPublishedItemInfo item)
        {
            AddToQueryHashTable(item, this.Stack.LocalQueryHashTable);
        }

        private void RebuildQueryHashTable()
        {
            try
            {
                this.Stack.LocalQueryHashTable.BeginUpdate();

                var newQueryHashTable = new QueryHashTable();
                PublishedItems.ForEach(item => AddToQueryHashTable(item, newQueryHashTable));

                this.Stack.LocalQueryHashTable.Set(newQueryHashTable);
            }
            catch
            {
                System.Diagnostics.Trace.WriteLine(String.Format("Failed to rebuild the query hash table."));
            }
            finally
            {
                this.Stack.LocalQueryHashTable.EndUpdate();
            }
        }

        public static QueryResult CreateQueryResult(
            Stack stack, Query query, Message extraInfo)
        {
            // TODO: Move this logic into KnownHubList (GetConnectedHub?)
            // Find a known hub that could be used as a relay.
            NodeInfo connectedNodeInfo = null;
            foreach (NodeInfo nodeInfo in stack.KnownHubList.ConnectedHubs)
            {
                connectedNodeInfo = nodeInfo;
                break;
            }

            QueryResult queryResult =
                new QueryResult(query.Guid, query.OriginNode,
                stack.NodeInfo, connectedNodeInfo);
            queryResult.ExtraInfo = extraInfo;
            queryResult.RequiresDirectConnection = false;
            return queryResult;
        }

        public static class Diagnostics
        {
            public static TimeSpan QueryHitProcessingTimeThreshold { get; set; }
            public static TimeSpan PublishItemMatchTimeThreshold { get; set; }

            static Diagnostics()
            {
                PublishItemMatchTimeThreshold = TimeSpan.FromSeconds(20);
                QueryHitProcessingTimeThreshold = TimeSpan.FromSeconds(5);
            }
        }
        

        /// <summary>
        /// Handles the Query Hit Event on the stack and sends out Query Results
        /// if any of the files that are being published match the query coming across
        /// PyxNet.
        /// </summary>
        /// <param name="stack">The stack to reply on. (Currently ignored!)</param>
        /// <param name="query">The Query that came in.</param>
        private void HandleQueryHit(Stack stack, Query query)
        {
            if (stack == null)
            {
                System.Diagnostics.Trace.WriteLine(String.Format("Cannot query using a null stack."));
                return; 
            }

            if (stack != this.Stack)
            {
                // TODO: We might want to return false immediately if the query
                // came from a different stack.  (As a security/privacy issue.)
                System.Diagnostics.Trace.WriteLine(String.Format(
                    "Unexpected query received on stack {0} instead of stack {1}.",
                    stack.NodeInfo.FriendlyName, this.Stack.NodeInfo.FriendlyName));
            }

            var watch = new System.Diagnostics.Stopwatch();

            watch.Start();

            PublishedItems.ForEach(
                delegate(IPublishedItemInfo item)
                {
                    try
                    {
                        var itemWatch = new System.Diagnostics.Stopwatch();
                        itemWatch.Start();
                        
                        QueryResult match = item.Matches(query, stack);

                        itemWatch.Stop();

                        if (itemWatch.Elapsed > Diagnostics.PublishItemMatchTimeThreshold)
                        {
                            Logging.Categories.Publishing.Warning("item.Match took more than " + itemWatch.Elapsed.TotalSeconds + "[sec] : query " + query.Guid + ", item: " + item.ToString());
                        }

                        if (match != null)
                        {
                            stack.ProcessQueryResult(null, match);
                        }
                    }
                    catch (Exception e)
                    {
                        Logging.Categories.Publishing.Error(e);
                        System.Diagnostics.Trace.WriteLine(String.Format("error occured while matching published item: {0} with error {1}",item,e.Message));
                    }
                }
            );

            watch.Stop();
            if (watch.Elapsed > Diagnostics.QueryHitProcessingTimeThreshold)
            {
                Logging.Categories.Publishing.Warning("Query processing took more then " + watch.Elapsed.TotalSeconds + "[sec] : " + query.Guid);
            }
        }

        private void OnKeywordsChanged(object sender, EventArgs e)
        {
            // If this is too slow, the non-dynamic items QHT can be stored and only dynamic items are re-hashed
            RebuildQueryHashTable();
        }
    }
}