namespace TokensExtractor
{
    partial class AppxURLs
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
            this.lvAppxURLs = new System.Windows.Forms.ListView();
            this.columnAppxURL = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnDLFolder = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.buttonCopyURL = new System.Windows.Forms.Button();
            this.buttonDownload = new System.Windows.Forms.Button();
            this.buttonClose = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // lvAppxURLs
            // 
            this.lvAppxURLs.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.lvAppxURLs.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnAppxURL,
            this.columnDLFolder});
            this.lvAppxURLs.FullRowSelect = true;
            this.lvAppxURLs.Location = new System.Drawing.Point(4, 4);
            this.lvAppxURLs.MultiSelect = false;
            this.lvAppxURLs.Name = "lvAppxURLs";
            this.lvAppxURLs.Size = new System.Drawing.Size(727, 569);
            this.lvAppxURLs.TabIndex = 0;
            this.lvAppxURLs.UseCompatibleStateImageBehavior = false;
            this.lvAppxURLs.View = System.Windows.Forms.View.Details;
            this.lvAppxURLs.SelectedIndexChanged += new System.EventHandler(this.lvAppxURLs_SelectedIndexChanged);
            // 
            // columnAppxURL
            // 
            this.columnAppxURL.Text = "Appx URL";
            this.columnAppxURL.Width = 350;
            // 
            // columnDLFolder
            // 
            this.columnDLFolder.Text = "Download folder";
            this.columnDLFolder.Width = 350;
            // 
            // buttonCopyURL
            // 
            this.buttonCopyURL.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonCopyURL.Enabled = false;
            this.buttonCopyURL.Location = new System.Drawing.Point(742, 34);
            this.buttonCopyURL.Name = "buttonCopyURL";
            this.buttonCopyURL.Size = new System.Drawing.Size(109, 33);
            this.buttonCopyURL.TabIndex = 1;
            this.buttonCopyURL.Text = "Copy URL";
            this.buttonCopyURL.UseVisualStyleBackColor = true;
            this.buttonCopyURL.Click += new System.EventHandler(this.buttonCopyURL_Click);
            // 
            // buttonDownload
            // 
            this.buttonDownload.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonDownload.Enabled = false;
            this.buttonDownload.Location = new System.Drawing.Point(742, 85);
            this.buttonDownload.Name = "buttonDownload";
            this.buttonDownload.Size = new System.Drawing.Size(109, 33);
            this.buttonDownload.TabIndex = 2;
            this.buttonDownload.Text = "Download";
            this.buttonDownload.UseVisualStyleBackColor = true;
            this.buttonDownload.Click += new System.EventHandler(this.buttonDownload_Click);
            // 
            // buttonClose
            // 
            this.buttonClose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonClose.Location = new System.Drawing.Point(742, 167);
            this.buttonClose.Name = "buttonClose";
            this.buttonClose.Size = new System.Drawing.Size(109, 33);
            this.buttonClose.TabIndex = 3;
            this.buttonClose.Text = "&Close";
            this.buttonClose.UseVisualStyleBackColor = true;
            this.buttonClose.Click += new System.EventHandler(this.buttonClose_Click);
            // 
            // AppxURLs
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonClose;
            this.ClientSize = new System.Drawing.Size(858, 577);
            this.Controls.Add(this.buttonClose);
            this.Controls.Add(this.buttonDownload);
            this.Controls.Add(this.buttonCopyURL);
            this.Controls.Add(this.lvAppxURLs);
            this.Name = "AppxURLs";
            this.Text = "Appx download URLs";
            this.Load += new System.EventHandler(this.AppxURLs_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListView lvAppxURLs;
        private System.Windows.Forms.Button buttonCopyURL;
        private System.Windows.Forms.Button buttonDownload;
        private System.Windows.Forms.Button buttonClose;
        private System.Windows.Forms.ColumnHeader columnAppxURL;
        private System.Windows.Forms.ColumnHeader columnDLFolder;
    }
}