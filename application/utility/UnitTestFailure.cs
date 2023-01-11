using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using Pyxis.Utilities;

namespace ApplicationUtility
{
    public partial class UnitTestFailure : Form
    {
        private Dictionary<string, ListViewItem> m_TestsItems = new Dictionary<string, ListViewItem>();
        private object m_closeLock = new object();
        private bool m_closed = false;

        public UnitTestFailure()
        {
            InitializeComponent();
        }

        public bool ShowFailedTestOnly { get; set; }

        public bool CloseIfCompletedWithNoFailures
        {
            get
            {
                return closeIfNoFailedUnitTestsCheckBox.Checked;
            }
            set
            {
                closeIfNoFailedUnitTestsCheckBox.Checked = value;
            }
        }

        private bool m_foundFailedTests = false;

        private void SafeClose()
        {
            lock (m_closeLock)
            {
                m_closed = true;
                Close();
            }
        }

        private void CloseButton_Click(object sender, EventArgs e)
        {
            SafeClose();
        }

        private void UnitTestFailure_Load(object sender, EventArgs e)
        {
            if (ShowFailedTestOnly)
            {
                foreach (var name in TestFrame.getInstance().getFailedTests())
                {
                    var item = new ListViewItem(new string[] { name, "", "Failed", "" });
                    TestsListView.Items.Add(item);
                    m_TestsItems[name] = item;
                }
            }
            else
            {
                foreach (var name in TestFrame.getInstance().getTests())
                {
                    var item = new ListViewItem(new string[] {name, "", "", ""});
                    TestsListView.Items.Add(item);
                    m_TestsItems[name] = item;
                }
                RunAllTests();
            }
        }

        private void TestsListView_DoubleClick(object sender, EventArgs e)
        {
            if (TestsListView.SelectedIndices.Count >= 0)
            {
                var item = TestsListView.SelectedItems[0];
                var name = item.Text;

                RunTest(name);
            }
        }

        private void RunAllButton_Click(object sender, EventArgs e)
        {
            System.Threading.ThreadPool.QueueUserWorkItem(RunAllTests, m_TestsItems.Keys);
        }

        public void RunAllTests()
        {
            if (m_TestsItems.Count == 0)
            {
                foreach (var name in TestFrame.getInstance().getTests())
                {
                    var item = new ListViewItem(new string[] { name, "", "", "" });
                    TestsListView.Items.Add(item);
                    m_TestsItems[name] = item;
                }
            }
            System.Threading.ThreadPool.QueueUserWorkItem(RunAllTests, m_TestsItems.Keys);            
        }

        private void RunAllTests(object state)
        {
            m_foundFailedTests = false;
            var items = state as IEnumerable<string>;
            foreach (var name in items)
            {
                RunTest(name);
            }

            if (!m_foundFailedTests && CloseIfCompletedWithNoFailures)
            {
                this.InvokeIfRequired(() => { SafeClose(); });
            }
        }

        private void RunTest(string name)
        {
            var item = m_TestsItems[name];

            this.InvokeIfSafe(() =>
                {
                    if (IsDisposed) return;

                    item.SubItems[2].Text = "Running";
                    item.EnsureVisible();
                }
            );

            var start = DateTime.Now;
            var failed = false;
            var error = false;

            try
            {
                if (!TestFrame.getInstance().test(name))
                {
                    failed = true;
                }
            }
            catch (Exception)
            {
                failed = true;
                error = true;
            }

            var totalTime = DateTime.Now - start;

            if (failed)
            {
                m_foundFailedTests = true;
            }


            this.InvokeIfSafe(() =>
            {
                if (IsDisposed) return;

                item.SubItems[3].Text = String.Format("{0:0.00} sec", totalTime.TotalSeconds);

                if (totalTime.TotalSeconds > 0.5)
                {
                    item.BackColor = Color.Khaki;
                }

                if (failed)
                {
                    item.SubItems[2].Text = error ? "Error" : "Failed";
                    item.ForeColor = Color.Red;
                }
                else
                {
                    item.SubItems[2].Text = "Passed";
                    item.ForeColor = Color.Green;
                }
            });
        }

        private void TestsListView_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (TestsListView.SelectedItems.Count > 0)
            {
                var lineStynax = new Regex(@"\[(.*)\]\s(.*)\s\((\d+)\)\s:\s(.*)");

                var item = TestsListView.SelectedItems[0];

                AssertsListView.Items.Clear();

                foreach(var line in TestFrame.getInstance().getTestLog(item.Text))
                {
                    var match = lineStynax.Match(line);

                    if (!match.Success) continue;

                    var logItem = new ListViewItem(new []
                                                       {
                                                           match.Groups[1].Value,
                                                           match.Groups[3].Value,
                                                           match.Groups[4].Value
                                                       });
                    AssertsListView.Items.Add(logItem);
                }
            }
        }

        private void InvokeIfSafe(Action action)
        {
            lock (m_closeLock)
            {
                if (IsHandleCreated && !m_closed)
                {
                    this.InvokeIfRequired(action);
                }
            }
        }
    }
}
