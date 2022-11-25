namespace TokensExtractor
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.lbPFM = new System.Windows.Forms.ListBox();
            this.buttonView = new System.Windows.Forms.Button();
            this.buttonClose = new System.Windows.Forms.Button();
            this.buttonCrackIt = new System.Windows.Forms.Button();
            this.buttonDigWuauservLog = new System.Windows.Forms.Button();
            this.buttonDelDLCache = new System.Windows.Forms.Button();
            this.buttonInstLicFromFile = new System.Windows.Forms.Button();
            this.buttonRefresh = new System.Windows.Forms.Button();
            this.buttonUninstLic = new System.Windows.Forms.Button();
            this.buttonFixStorewuauthTokensCache = new System.Windows.Forms.Button();
            this.buttonBruteDig = new System.Windows.Forms.Button();
            this.buttonSearch = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // lbPFM
            // 
            this.lbPFM.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.lbPFM.FormattingEnabled = true;
            this.lbPFM.ItemHeight = 16;
            this.lbPFM.Location = new System.Drawing.Point(5, 2);
            this.lbPFM.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.lbPFM.Name = "lbPFM";
            this.lbPFM.Size = new System.Drawing.Size(671, 676);
            this.lbPFM.TabIndex = 0;
            this.lbPFM.SelectedIndexChanged += new System.EventHandler(this.lbPFM_SelectedIndexChanged);
            this.lbPFM.DoubleClick += new System.EventHandler(this.buttonView_Click);
            // 
            // buttonView
            // 
            this.buttonView.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonView.Enabled = false;
            this.buttonView.Location = new System.Drawing.Point(685, 158);
            this.buttonView.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.buttonView.Name = "buttonView";
            this.buttonView.Size = new System.Drawing.Size(177, 39);
            this.buttonView.TabIndex = 4;
            this.buttonView.Text = "&View";
            this.buttonView.UseVisualStyleBackColor = true;
            this.buttonView.Click += new System.EventHandler(this.buttonView_Click);
            // 
            // buttonClose
            // 
            this.buttonClose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonClose.Location = new System.Drawing.Point(685, 632);
            this.buttonClose.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.buttonClose.Name = "buttonClose";
            this.buttonClose.Size = new System.Drawing.Size(177, 39);
            this.buttonClose.TabIndex = 11;
            this.buttonClose.Text = "Close";
            this.buttonClose.UseVisualStyleBackColor = true;
            this.buttonClose.Click += new System.EventHandler(this.buttonClose_Click);
            // 
            // buttonCrackIt
            // 
            this.buttonCrackIt.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonCrackIt.Enabled = false;
            this.buttonCrackIt.Location = new System.Drawing.Point(685, 206);
            this.buttonCrackIt.Margin = new System.Windows.Forms.Padding(4);
            this.buttonCrackIt.Name = "buttonCrackIt";
            this.buttonCrackIt.Size = new System.Drawing.Size(179, 41);
            this.buttonCrackIt.TabIndex = 5;
            this.buttonCrackIt.Text = "CrackIt !";
            this.buttonCrackIt.UseVisualStyleBackColor = true;
            this.buttonCrackIt.Click += new System.EventHandler(this.buttonCrackIt_Click);
            // 
            // buttonDigWuauservLog
            // 
            this.buttonDigWuauservLog.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonDigWuauservLog.Location = new System.Drawing.Point(684, 409);
            this.buttonDigWuauservLog.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.buttonDigWuauservLog.Name = "buttonDigWuauservLog";
            this.buttonDigWuauservLog.Size = new System.Drawing.Size(179, 57);
            this.buttonDigWuauservLog.TabIndex = 8;
            this.buttonDigWuauservLog.Text = "Dig for .appx URLs";
            this.buttonDigWuauservLog.UseVisualStyleBackColor = true;
            this.buttonDigWuauservLog.Click += new System.EventHandler(this.buttonDigWuauservLog_Click);
            // 
            // buttonDelDLCache
            // 
            this.buttonDelDLCache.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonDelDLCache.Location = new System.Drawing.Point(684, 476);
            this.buttonDelDLCache.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.buttonDelDLCache.Name = "buttonDelDLCache";
            this.buttonDelDLCache.Size = new System.Drawing.Size(179, 64);
            this.buttonDelDLCache.TabIndex = 9;
            this.buttonDelDLCache.Text = "Delete download cache";
            this.buttonDelDLCache.UseVisualStyleBackColor = true;
            this.buttonDelDLCache.Click += new System.EventHandler(this.buttonDelDLCache_Click);
            // 
            // buttonInstLicFromFile
            // 
            this.buttonInstLicFromFile.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonInstLicFromFile.Location = new System.Drawing.Point(684, 268);
            this.buttonInstLicFromFile.Margin = new System.Windows.Forms.Padding(4);
            this.buttonInstLicFromFile.Name = "buttonInstLicFromFile";
            this.buttonInstLicFromFile.Size = new System.Drawing.Size(179, 57);
            this.buttonInstLicFromFile.TabIndex = 6;
            this.buttonInstLicFromFile.Text = "Install lic from a file";
            this.buttonInstLicFromFile.UseVisualStyleBackColor = true;
            this.buttonInstLicFromFile.Click += new System.EventHandler(this.buttonInstLicFromFile_Click);
            // 
            // buttonRefresh
            // 
            this.buttonRefresh.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonRefresh.Location = new System.Drawing.Point(685, 10);
            this.buttonRefresh.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.buttonRefresh.Name = "buttonRefresh";
            this.buttonRefresh.Size = new System.Drawing.Size(177, 34);
            this.buttonRefresh.TabIndex = 1;
            this.buttonRefresh.Text = "&Refresh";
            this.buttonRefresh.UseVisualStyleBackColor = true;
            this.buttonRefresh.Click += new System.EventHandler(this.buttonRefresh_Click);
            // 
            // buttonUninstLic
            // 
            this.buttonUninstLic.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonUninstLic.Enabled = false;
            this.buttonUninstLic.Location = new System.Drawing.Point(684, 334);
            this.buttonUninstLic.Margin = new System.Windows.Forms.Padding(4);
            this.buttonUninstLic.Name = "buttonUninstLic";
            this.buttonUninstLic.Size = new System.Drawing.Size(179, 39);
            this.buttonUninstLic.TabIndex = 7;
            this.buttonUninstLic.Text = "Uninstall lic";
            this.buttonUninstLic.UseVisualStyleBackColor = true;
            this.buttonUninstLic.Click += new System.EventHandler(this.buttonUninstLic_Click);
            // 
            // buttonFixStorewuauthTokensCache
            // 
            this.buttonFixStorewuauthTokensCache.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonFixStorewuauthTokensCache.Location = new System.Drawing.Point(685, 545);
            this.buttonFixStorewuauthTokensCache.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.buttonFixStorewuauthTokensCache.Name = "buttonFixStorewuauthTokensCache";
            this.buttonFixStorewuauthTokensCache.Size = new System.Drawing.Size(177, 58);
            this.buttonFixStorewuauthTokensCache.TabIndex = 10;
            this.buttonFixStorewuauthTokensCache.Text = "Fix error 0x8024600e";
            this.buttonFixStorewuauthTokensCache.UseVisualStyleBackColor = true;
            this.buttonFixStorewuauthTokensCache.Click += new System.EventHandler(this.buttonFixStorewuauthTokensCache_Click);
            // 
            // buttonBruteDig
            // 
            this.buttonBruteDig.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonBruteDig.Location = new System.Drawing.Point(685, 50);
            this.buttonBruteDig.Margin = new System.Windows.Forms.Padding(4);
            this.buttonBruteDig.Name = "buttonBruteDig";
            this.buttonBruteDig.Size = new System.Drawing.Size(177, 47);
            this.buttonBruteDig.TabIndex = 2;
            this.buttonBruteDig.Text = "Bruteforce tokens.dat";
            this.buttonBruteDig.UseVisualStyleBackColor = true;
            this.buttonBruteDig.Click += new System.EventHandler(this.buttonBruteDig_Click);
            // 
            // buttonSearch
            // 
            this.buttonSearch.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonSearch.Location = new System.Drawing.Point(685, 110);
            this.buttonSearch.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.buttonSearch.Name = "buttonSearch";
            this.buttonSearch.Size = new System.Drawing.Size(177, 39);
            this.buttonSearch.TabIndex = 3;
            this.buttonSearch.Text = "&Search";
            this.buttonSearch.UseVisualStyleBackColor = true;
            this.buttonSearch.Click += new System.EventHandler(this.buttonSearch_Click);
            // 
            // Form1
            // 
            this.AcceptButton = this.buttonView;
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonClose;
            this.ClientSize = new System.Drawing.Size(869, 691);
            this.Controls.Add(this.buttonSearch);
            this.Controls.Add(this.buttonBruteDig);
            this.Controls.Add(this.buttonFixStorewuauthTokensCache);
            this.Controls.Add(this.buttonUninstLic);
            this.Controls.Add(this.buttonRefresh);
            this.Controls.Add(this.buttonInstLicFromFile);
            this.Controls.Add(this.buttonDelDLCache);
            this.Controls.Add(this.buttonDigWuauservLog);
            this.Controls.Add(this.buttonCrackIt);
            this.Controls.Add(this.buttonClose);
            this.Controls.Add(this.buttonView);
            this.Controls.Add(this.lbPFM);
            this.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.Name = "Form1";
            this.Text = "WSService tokens extractor v1.4.2";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListBox lbPFM;
        private System.Windows.Forms.Button buttonView;
        private System.Windows.Forms.Button buttonClose;
        private System.Windows.Forms.Button buttonCrackIt;
        private System.Windows.Forms.Button buttonDigWuauservLog;
        private System.Windows.Forms.Button buttonDelDLCache;
        private System.Windows.Forms.Button buttonInstLicFromFile;
        private System.Windows.Forms.Button buttonRefresh;
        private System.Windows.Forms.Button buttonUninstLic;
        private System.Windows.Forms.Button buttonFixStorewuauthTokensCache;
        private System.Windows.Forms.Button buttonBruteDig;
        private System.Windows.Forms.Button buttonSearch;
    }
}

