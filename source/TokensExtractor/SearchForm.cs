using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace TokensExtractor
{
    public partial class SearchForm : Form
    {

        public SearchForm()
        {
            InitializeComponent();
        }

        public string searchtext;
        private void buttonOK_Click(object sender, EventArgs e)
        {
            searchtext = textSearch.Text;
        }

        private void SearchForm_Load(object sender, EventArgs e)
        {
            CenterToParent();
        }

        private void SearchForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (DialogResult == DialogResult.OK && searchtext.Length == 0)
            {
                e.Cancel = true;
                MessageBox.Show("Must enter some text","Search");
                textSearch.Focus();
            }
        }
    }
}
