namespace RobobuilderLib
{
    partial class Basic_frm
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
            if (editfl)
            {
                if (System.Windows.Forms.MessageBox.Show("File changed but not saved?", "Save", System.Windows.Forms.MessageBoxButtons.OKCancel) == System.Windows.Forms.DialogResult.Cancel)
                    return;
            }

            writeini(); //save - this should the registry really

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
            this.components = new System.ComponentModel.Container();
            this.output = new System.Windows.Forms.TextBox();
            this.comPort = new System.Windows.Forms.TextBox();
            this.button1 = new System.Windows.Forms.Button();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.newToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loadToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.reLoadToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loadBinaryToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveAsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.compilerToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.compileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.downloadToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.extendToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.simulatorToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.imageToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fname = new System.Windows.Forms.TextBox();
            this.progressBar1 = new System.Windows.Forms.ProgressBar();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.input = new System.Windows.Forms.RichTextBox();
            this.helptext = new System.Windows.Forms.Label();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.stopBtn = new System.Windows.Forms.Button();
            this.listBtn = new System.Windows.Forms.Button();
            this.runBtn = new System.Windows.Forms.Button();
            this.comselect = new System.Windows.Forms.ListBox();
            this.label1 = new System.Windows.Forms.Label();
            this.term = new System.Windows.Forms.TextBox();
            this.tabPage4 = new System.Windows.Forms.TabPage();
            this.textBox3 = new System.Windows.Forms.TextBox();
            this.checkBox2 = new System.Windows.Forms.CheckBox();
            this.checkBox1 = new System.Windows.Forms.CheckBox();
            this.textBox2 = new System.Windows.Forms.TextBox();
            this.label11 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.filtercb = new System.Windows.Forms.CheckBox();
            this.ldWeb = new System.Windows.Forms.Button();
            this.ldFile = new System.Windows.Forms.Button();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.button2 = new System.Windows.Forms.Button();
            this.button3 = new System.Windows.Forms.Button();
            this.button4 = new System.Windows.Forms.Button();
            this.button5 = new System.Windows.Forms.Button();
            this.button6 = new System.Windows.Forms.Button();
            this.button7 = new System.Windows.Forms.Button();
            this.button8 = new System.Windows.Forms.Button();
            this.button9 = new System.Windows.Forms.Button();
            this.button10 = new System.Windows.Forms.Button();
            this.button11 = new System.Windows.Forms.Button();
            this.button12 = new System.Windows.Forms.Button();
            this.button13 = new System.Windows.Forms.Button();
            this.button14 = new System.Windows.Forms.Button();
            this.button15 = new System.Windows.Forms.Button();
            this.button16 = new System.Windows.Forms.Button();
            this.button17 = new System.Windows.Forms.Button();
            this.button18 = new System.Windows.Forms.Button();
            this.button19 = new System.Windows.Forms.Button();
            this.button20 = new System.Windows.Forms.Button();
            this.button21 = new System.Windows.Forms.Button();
            this.button22 = new System.Windows.Forms.Button();
            this.button23 = new System.Windows.Forms.Button();
            this.button24 = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.label12 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.menuStrip1.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.tabPage3.SuspendLayout();
            this.tabPage4.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // output
            // 
            this.output.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(192)))), ((int)(((byte)(128)))));
            this.output.Location = new System.Drawing.Point(3, 3);
            this.output.Multiline = true;
            this.output.Name = "output";
            this.output.ReadOnly = true;
            this.output.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.output.Size = new System.Drawing.Size(769, 384);
            this.output.TabIndex = 4;
            // 
            // comPort
            // 
            this.comPort.Location = new System.Drawing.Point(176, 363);
            this.comPort.Name = "comPort";
            this.comPort.Size = new System.Drawing.Size(87, 20);
            this.comPort.TabIndex = 9;
            this.comPort.Text = "COM5";
            this.comPort.Visible = false;
            // 
            // button1
            // 
            this.button1.BackColor = System.Drawing.Color.Red;
            this.button1.Location = new System.Drawing.Point(738, 362);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(42, 24);
            this.button1.TabIndex = 10;
            this.button1.Text = "Con";
            this.button1.UseVisualStyleBackColor = false;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.compilerToolStripMenuItem,
            this.helpToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(796, 24);
            this.menuStrip1.TabIndex = 11;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.newToolStripMenuItem,
            this.loadToolStripMenuItem,
            this.reLoadToolStripMenuItem,
            this.loadBinaryToolStripMenuItem,
            this.saveToolStripMenuItem,
            this.saveAsToolStripMenuItem,
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // newToolStripMenuItem
            // 
            this.newToolStripMenuItem.Name = "newToolStripMenuItem";
            this.newToolStripMenuItem.Size = new System.Drawing.Size(136, 22);
            this.newToolStripMenuItem.Text = "New";
            this.newToolStripMenuItem.Click += new System.EventHandler(this.newToolStripMenuItem_Click);
            // 
            // loadToolStripMenuItem
            // 
            this.loadToolStripMenuItem.Name = "loadToolStripMenuItem";
            this.loadToolStripMenuItem.Size = new System.Drawing.Size(136, 22);
            this.loadToolStripMenuItem.Text = "Load";
            this.loadToolStripMenuItem.Click += new System.EventHandler(this.loadToolStripMenuItem_Click);
            // 
            // reLoadToolStripMenuItem
            // 
            this.reLoadToolStripMenuItem.Name = "reLoadToolStripMenuItem";
            this.reLoadToolStripMenuItem.Size = new System.Drawing.Size(136, 22);
            this.reLoadToolStripMenuItem.Text = "ReLoad";
            this.reLoadToolStripMenuItem.Click += new System.EventHandler(this.reLoadToolStripMenuItem_Click);
            // 
            // loadBinaryToolStripMenuItem
            // 
            this.loadBinaryToolStripMenuItem.Name = "loadBinaryToolStripMenuItem";
            this.loadBinaryToolStripMenuItem.Size = new System.Drawing.Size(136, 22);
            this.loadBinaryToolStripMenuItem.Text = "Load Binary";
            this.loadBinaryToolStripMenuItem.Click += new System.EventHandler(this.loadBinaryToolStripMenuItem_Click);
            // 
            // saveToolStripMenuItem
            // 
            this.saveToolStripMenuItem.Name = "saveToolStripMenuItem";
            this.saveToolStripMenuItem.Size = new System.Drawing.Size(136, 22);
            this.saveToolStripMenuItem.Text = "Save";
            this.saveToolStripMenuItem.Click += new System.EventHandler(this.saveToolStripMenuItem_Click);
            // 
            // saveAsToolStripMenuItem
            // 
            this.saveAsToolStripMenuItem.Name = "saveAsToolStripMenuItem";
            this.saveAsToolStripMenuItem.Size = new System.Drawing.Size(136, 22);
            this.saveAsToolStripMenuItem.Text = "Save As";
            this.saveAsToolStripMenuItem.Click += new System.EventHandler(this.saveAsToolStripMenuItem_Click);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(136, 22);
            this.exitToolStripMenuItem.Text = "Exit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // compilerToolStripMenuItem
            // 
            this.compilerToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.compileToolStripMenuItem,
            this.downloadToolStripMenuItem,
            this.extendToolStripMenuItem,
            this.simulatorToolStripMenuItem,
            this.imageToolStripMenuItem});
            this.compilerToolStripMenuItem.Name = "compilerToolStripMenuItem";
            this.compilerToolStripMenuItem.Size = new System.Drawing.Size(68, 20);
            this.compilerToolStripMenuItem.Text = "Compiler";
            // 
            // compileToolStripMenuItem
            // 
            this.compileToolStripMenuItem.Name = "compileToolStripMenuItem";
            this.compileToolStripMenuItem.Size = new System.Drawing.Size(141, 22);
            this.compileToolStripMenuItem.Text = "Compile";
            this.compileToolStripMenuItem.Click += new System.EventHandler(this.compileToolStripMenuItem_Click);
            // 
            // downloadToolStripMenuItem
            // 
            this.downloadToolStripMenuItem.Name = "downloadToolStripMenuItem";
            this.downloadToolStripMenuItem.Size = new System.Drawing.Size(141, 22);
            this.downloadToolStripMenuItem.Text = "Download";
            this.downloadToolStripMenuItem.Click += new System.EventHandler(this.downloadToolStripMenuItem_Click);
            // 
            // extendToolStripMenuItem
            // 
            this.extendToolStripMenuItem.Checked = true;
            this.extendToolStripMenuItem.CheckState = System.Windows.Forms.CheckState.Checked;
            this.extendToolStripMenuItem.Name = "extendToolStripMenuItem";
            this.extendToolStripMenuItem.Size = new System.Drawing.Size(141, 22);
            this.extendToolStripMenuItem.Text = "Firmware";
            this.extendToolStripMenuItem.Click += new System.EventHandler(this.extendToolStripMenuItem_Click);
            // 
            // simulatorToolStripMenuItem
            // 
            this.simulatorToolStripMenuItem.Name = "simulatorToolStripMenuItem";
            this.simulatorToolStripMenuItem.Size = new System.Drawing.Size(141, 22);
            this.simulatorToolStripMenuItem.Text = "Simulator";
            this.simulatorToolStripMenuItem.Click += new System.EventHandler(this.simulatorToolStripMenuItem_Click);
            // 
            // imageToolStripMenuItem
            // 
            this.imageToolStripMenuItem.Name = "imageToolStripMenuItem";
            this.imageToolStripMenuItem.Size = new System.Drawing.Size(141, 22);
            this.imageToolStripMenuItem.Text = "Image Mode";
            this.imageToolStripMenuItem.Click += new System.EventHandler(this.imageToolStripMenuItem_Click);
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutToolStripMenuItem});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
            this.helpToolStripMenuItem.Text = "Help";
            // 
            // aboutToolStripMenuItem
            // 
            this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
            this.aboutToolStripMenuItem.Size = new System.Drawing.Size(107, 22);
            this.aboutToolStripMenuItem.Text = "About";
            this.aboutToolStripMenuItem.Click += new System.EventHandler(this.aboutToolStripMenuItem_Click);
            // 
            // fname
            // 
            this.fname.Location = new System.Drawing.Point(0, 370);
            this.fname.Name = "fname";
            this.fname.Size = new System.Drawing.Size(777, 20);
            this.fname.TabIndex = 12;
            // 
            // progressBar1
            // 
            this.progressBar1.ForeColor = System.Drawing.Color.HotPink;
            this.progressBar1.Location = new System.Drawing.Point(8, 365);
            this.progressBar1.Name = "progressBar1";
            this.progressBar1.Size = new System.Drawing.Size(159, 19);
            this.progressBar1.TabIndex = 13;
            this.progressBar1.Visible = false;
            // 
            // timer1
            // 
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // input
            // 
            this.input.Font = new System.Drawing.Font("Courier New", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.input.Location = new System.Drawing.Point(0, 0);
            this.input.Name = "input";
            this.input.Size = new System.Drawing.Size(777, 367);
            this.input.TabIndex = 14;
            this.input.Text = "";
            this.input.SelectionChanged += new System.EventHandler(this.input_SelectionChanged);
            this.input.TextChanged += new System.EventHandler(this.input_TextChanged);
            this.input.MouseLeave += new System.EventHandler(this.input_MouseLeave);
            this.input.MouseHover += new System.EventHandler(this.input_MouseHover);
            this.input.MouseMove += new System.Windows.Forms.MouseEventHandler(this.input_MouseMove);
            // 
            // helptext
            // 
            this.helptext.AutoSize = true;
            this.helptext.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(255)))), ((int)(((byte)(192)))));
            this.helptext.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.helptext.Location = new System.Drawing.Point(339, 34);
            this.helptext.Name = "helptext";
            this.helptext.Size = new System.Drawing.Size(37, 15);
            this.helptext.TabIndex = 15;
            this.helptext.Text = "label1";
            this.helptext.Visible = false;
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Controls.Add(this.tabPage3);
            this.tabControl1.Controls.Add(this.tabPage4);
            this.tabControl1.Location = new System.Drawing.Point(0, 27);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(788, 419);
            this.tabControl1.TabIndex = 16;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.input);
            this.tabPage1.Controls.Add(this.fname);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(780, 393);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "editor";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.output);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(780, 393);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Output";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.stopBtn);
            this.tabPage3.Controls.Add(this.listBtn);
            this.tabPage3.Controls.Add(this.runBtn);
            this.tabPage3.Controls.Add(this.comselect);
            this.tabPage3.Controls.Add(this.label1);
            this.tabPage3.Controls.Add(this.term);
            this.tabPage3.Controls.Add(this.button1);
            this.tabPage3.Controls.Add(this.progressBar1);
            this.tabPage3.Controls.Add(this.comPort);
            this.tabPage3.Location = new System.Drawing.Point(4, 22);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Size = new System.Drawing.Size(780, 393);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "VT100";
            this.tabPage3.UseVisualStyleBackColor = true;
            // 
            // stopBtn
            // 
            this.stopBtn.BackColor = System.Drawing.Color.YellowGreen;
            this.stopBtn.Location = new System.Drawing.Point(521, 362);
            this.stopBtn.Name = "stopBtn";
            this.stopBtn.Size = new System.Drawing.Size(47, 21);
            this.stopBtn.TabIndex = 18;
            this.stopBtn.Text = "STOP";
            this.stopBtn.UseVisualStyleBackColor = false;
            this.stopBtn.Visible = false;
            this.stopBtn.Click += new System.EventHandler(this.stopBtn_Click);
            // 
            // listBtn
            // 
            this.listBtn.BackColor = System.Drawing.Color.YellowGreen;
            this.listBtn.Location = new System.Drawing.Point(472, 363);
            this.listBtn.Name = "listBtn";
            this.listBtn.Size = new System.Drawing.Size(43, 21);
            this.listBtn.TabIndex = 17;
            this.listBtn.Text = "LIST";
            this.listBtn.UseVisualStyleBackColor = false;
            this.listBtn.Visible = false;
            this.listBtn.Click += new System.EventHandler(this.listBtn_Click);
            // 
            // runBtn
            // 
            this.runBtn.BackColor = System.Drawing.Color.YellowGreen;
            this.runBtn.Location = new System.Drawing.Point(423, 363);
            this.runBtn.Name = "runBtn";
            this.runBtn.Size = new System.Drawing.Size(43, 21);
            this.runBtn.TabIndex = 16;
            this.runBtn.Text = "RUN";
            this.runBtn.UseVisualStyleBackColor = false;
            this.runBtn.Visible = false;
            this.runBtn.Click += new System.EventHandler(this.runBtn_Click);
            // 
            // comselect
            // 
            this.comselect.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.comselect.FormattingEnabled = true;
            this.comselect.Location = new System.Drawing.Point(586, 365);
            this.comselect.Name = "comselect";
            this.comselect.Size = new System.Drawing.Size(146, 15);
            this.comselect.TabIndex = 15;
            this.comselect.SelectedIndexChanged += new System.EventHandler(this.comselect_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(269, 370);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(69, 13);
            this.label1.TabIndex = 14;
            this.label1.Text = "FIRMWARE ";
            this.label1.Visible = false;
            // 
            // term
            // 
            this.term.BackColor = System.Drawing.Color.Black;
            this.term.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.term.ForeColor = System.Drawing.Color.Green;
            this.term.Location = new System.Drawing.Point(4, 2);
            this.term.Multiline = true;
            this.term.Name = "term";
            this.term.ReadOnly = true;
            this.term.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.term.Size = new System.Drawing.Size(768, 357);
            this.term.TabIndex = 0;
            this.term.Text = "Connect to robot";
            // 
            // tabPage4
            // 
            this.tabPage4.Controls.Add(this.textBox3);
            this.tabPage4.Controls.Add(this.checkBox2);
            this.tabPage4.Controls.Add(this.checkBox1);
            this.tabPage4.Controls.Add(this.textBox2);
            this.tabPage4.Controls.Add(this.label11);
            this.tabPage4.Controls.Add(this.label10);
            this.tabPage4.Controls.Add(this.label3);
            this.tabPage4.Controls.Add(this.label9);
            this.tabPage4.Controls.Add(this.label8);
            this.tabPage4.Controls.Add(this.label7);
            this.tabPage4.Controls.Add(this.label6);
            this.tabPage4.Controls.Add(this.label5);
            this.tabPage4.Controls.Add(this.label4);
            this.tabPage4.Controls.Add(this.textBox1);
            this.tabPage4.Controls.Add(this.filtercb);
            this.tabPage4.Controls.Add(this.ldWeb);
            this.tabPage4.Controls.Add(this.ldFile);
            this.tabPage4.Controls.Add(this.pictureBox1);
            this.tabPage4.Location = new System.Drawing.Point(4, 22);
            this.tabPage4.Name = "tabPage4";
            this.tabPage4.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage4.Size = new System.Drawing.Size(780, 393);
            this.tabPage4.TabIndex = 3;
            this.tabPage4.Text = "Image";
            this.tabPage4.UseVisualStyleBackColor = true;
            // 
            // textBox3
            // 
            this.textBox3.Location = new System.Drawing.Point(523, 248);
            this.textBox3.Name = "textBox3";
            this.textBox3.Size = new System.Drawing.Size(249, 20);
            this.textBox3.TabIndex = 18;
            // 
            // checkBox2
            // 
            this.checkBox2.AutoSize = true;
            this.checkBox2.Location = new System.Drawing.Point(524, 123);
            this.checkBox2.Name = "checkBox2";
            this.checkBox2.Size = new System.Drawing.Size(72, 17);
            this.checkBox2.TabIndex = 17;
            this.checkBox2.Text = "Normalise";
            this.checkBox2.UseVisualStyleBackColor = true;
            this.checkBox2.CheckedChanged += new System.EventHandler(this.checkBox2_CheckedChanged);
            // 
            // checkBox1
            // 
            this.checkBox1.AutoSize = true;
            this.checkBox1.Location = new System.Drawing.Point(572, 328);
            this.checkBox1.Name = "checkBox1";
            this.checkBox1.Size = new System.Drawing.Size(48, 17);
            this.checkBox1.TabIndex = 16;
            this.checkBox1.Text = "Grey";
            this.checkBox1.UseVisualStyleBackColor = true;
            this.checkBox1.CheckedChanged += new System.EventHandler(this.checkBox1_CheckedChanged_1);
            // 
            // textBox2
            // 
            this.textBox2.Location = new System.Drawing.Point(515, 326);
            this.textBox2.Name = "textBox2";
            this.textBox2.Size = new System.Drawing.Size(44, 20);
            this.textBox2.TabIndex = 15;
            this.textBox2.Text = "32";
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(720, 96);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(27, 13);
            this.label11.TabIndex = 14;
            this.label11.Text = "Red";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(720, 72);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(36, 13);
            this.label10.TabIndex = 13;
            this.label10.Text = "Green";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(720, 47);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(28, 13);
            this.label3.TabIndex = 12;
            this.label3.Text = "Blue";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(669, 96);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(35, 13);
            this.label9.TabIndex = 10;
            this.label9.Text = "label2";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(669, 72);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(35, 13);
            this.label8.TabIndex = 9;
            this.label8.Text = "label2";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(669, 47);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(35, 13);
            this.label7.TabIndex = 8;
            this.label7.Text = "label2";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(628, 96);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(35, 13);
            this.label6.TabIndex = 7;
            this.label6.Text = "label2";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(628, 72);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(35, 13);
            this.label5.TabIndex = 6;
            this.label5.Text = "label2";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(628, 47);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(35, 13);
            this.label4.TabIndex = 5;
            this.label4.Text = "label2";
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(514, 163);
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(260, 20);
            this.textBox1.TabIndex = 4;
            this.textBox1.Text = "http://robobuilderlib.googlecode.com/files/test-old.jpg";
            // 
            // filtercb
            // 
            this.filtercb.AutoSize = true;
            this.filtercb.Location = new System.Drawing.Point(630, 123);
            this.filtercb.Name = "filtercb";
            this.filtercb.Size = new System.Drawing.Size(74, 17);
            this.filtercb.TabIndex = 3;
            this.filtercb.Text = "Apply filter";
            this.filtercb.UseVisualStyleBackColor = true;
            this.filtercb.CheckedChanged += new System.EventHandler(this.checkBox1_CheckedChanged);
            // 
            // ldWeb
            // 
            this.ldWeb.Location = new System.Drawing.Point(711, 189);
            this.ldWeb.Name = "ldWeb";
            this.ldWeb.Size = new System.Drawing.Size(61, 25);
            this.ldWeb.TabIndex = 2;
            this.ldWeb.Text = "Load Web";
            this.ldWeb.UseVisualStyleBackColor = true;
            this.ldWeb.Click += new System.EventHandler(this.ldWeb_Click);
            // 
            // ldFile
            // 
            this.ldFile.Location = new System.Drawing.Point(524, 41);
            this.ldFile.Name = "ldFile";
            this.ldFile.Size = new System.Drawing.Size(61, 25);
            this.ldFile.TabIndex = 1;
            this.ldFile.Text = "Load file";
            this.ldFile.UseVisualStyleBackColor = true;
            this.ldFile.Click += new System.EventHandler(this.ldFile_Click);
            // 
            // pictureBox1
            // 
            this.pictureBox1.Location = new System.Drawing.Point(28, 25);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(467, 330);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            this.pictureBox1.Click += new System.EventHandler(this.pictureBox1_Click);
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "openFileDialog1";
            // 
            // button2
            // 
            this.button2.BackColor = System.Drawing.Color.Silver;
            this.button2.Location = new System.Drawing.Point(41, 198);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(35, 23);
            this.button2.TabIndex = 17;
            this.button2.Text = "1";
            this.button2.UseVisualStyleBackColor = false;
            // 
            // button3
            // 
            this.button3.BackColor = System.Drawing.Color.Silver;
            this.button3.Location = new System.Drawing.Point(82, 198);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(35, 23);
            this.button3.TabIndex = 18;
            this.button3.Text = "2";
            this.button3.UseVisualStyleBackColor = false;
            // 
            // button4
            // 
            this.button4.BackColor = System.Drawing.Color.Silver;
            this.button4.Location = new System.Drawing.Point(123, 198);
            this.button4.Name = "button4";
            this.button4.Size = new System.Drawing.Size(35, 23);
            this.button4.TabIndex = 19;
            this.button4.Text = "3";
            this.button4.UseVisualStyleBackColor = false;
            // 
            // button5
            // 
            this.button5.BackColor = System.Drawing.Color.Silver;
            this.button5.Location = new System.Drawing.Point(41, 227);
            this.button5.Name = "button5";
            this.button5.Size = new System.Drawing.Size(35, 23);
            this.button5.TabIndex = 20;
            this.button5.Text = "4";
            this.button5.UseVisualStyleBackColor = false;
            // 
            // button6
            // 
            this.button6.BackColor = System.Drawing.Color.Silver;
            this.button6.Location = new System.Drawing.Point(82, 227);
            this.button6.Name = "button6";
            this.button6.Size = new System.Drawing.Size(35, 23);
            this.button6.TabIndex = 21;
            this.button6.Text = "5";
            this.button6.UseVisualStyleBackColor = false;
            // 
            // button7
            // 
            this.button7.BackColor = System.Drawing.Color.Silver;
            this.button7.Location = new System.Drawing.Point(123, 227);
            this.button7.Name = "button7";
            this.button7.Size = new System.Drawing.Size(35, 23);
            this.button7.TabIndex = 22;
            this.button7.Text = "6";
            this.button7.UseVisualStyleBackColor = false;
            this.button7.Click += new System.EventHandler(this.button7_Click);
            // 
            // button8
            // 
            this.button8.BackColor = System.Drawing.Color.Silver;
            this.button8.Location = new System.Drawing.Point(41, 256);
            this.button8.Name = "button8";
            this.button8.Size = new System.Drawing.Size(35, 23);
            this.button8.TabIndex = 23;
            this.button8.Text = "7";
            this.button8.UseVisualStyleBackColor = false;
            // 
            // button9
            // 
            this.button9.BackColor = System.Drawing.Color.Silver;
            this.button9.Location = new System.Drawing.Point(82, 256);
            this.button9.Name = "button9";
            this.button9.Size = new System.Drawing.Size(35, 23);
            this.button9.TabIndex = 24;
            this.button9.Text = "8";
            this.button9.UseVisualStyleBackColor = false;
            // 
            // button10
            // 
            this.button10.BackColor = System.Drawing.Color.Silver;
            this.button10.Location = new System.Drawing.Point(123, 256);
            this.button10.Name = "button10";
            this.button10.Size = new System.Drawing.Size(35, 23);
            this.button10.TabIndex = 25;
            this.button10.Text = "9";
            this.button10.UseVisualStyleBackColor = false;
            // 
            // button11
            // 
            this.button11.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(192)))), ((int)(((byte)(192)))));
            this.button11.Location = new System.Drawing.Point(41, 285);
            this.button11.Name = "button11";
            this.button11.Size = new System.Drawing.Size(35, 23);
            this.button11.TabIndex = 26;
            this.button11.Text = "*";
            this.button11.UseVisualStyleBackColor = false;
            // 
            // button12
            // 
            this.button12.BackColor = System.Drawing.Color.Silver;
            this.button12.Location = new System.Drawing.Point(82, 285);
            this.button12.Name = "button12";
            this.button12.Size = new System.Drawing.Size(35, 23);
            this.button12.TabIndex = 27;
            this.button12.Text = "0";
            this.button12.UseVisualStyleBackColor = false;
            // 
            // button13
            // 
            this.button13.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(192)))), ((int)(((byte)(192)))));
            this.button13.Location = new System.Drawing.Point(123, 285);
            this.button13.Name = "button13";
            this.button13.Size = new System.Drawing.Size(35, 23);
            this.button13.TabIndex = 28;
            this.button13.Text = "#";
            this.button13.UseVisualStyleBackColor = false;
            // 
            // button14
            // 
            this.button14.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(192)))), ((int)(((byte)(192)))));
            this.button14.Location = new System.Drawing.Point(82, 67);
            this.button14.Name = "button14";
            this.button14.Size = new System.Drawing.Size(35, 23);
            this.button14.TabIndex = 29;
            this.button14.Text = "^";
            this.button14.UseVisualStyleBackColor = false;
            // 
            // button15
            // 
            this.button15.BackColor = System.Drawing.Color.Red;
            this.button15.Location = new System.Drawing.Point(82, 109);
            this.button15.Name = "button15";
            this.button15.Size = new System.Drawing.Size(35, 23);
            this.button15.TabIndex = 30;
            this.button15.Text = "[]";
            this.button15.UseVisualStyleBackColor = false;
            this.button15.Click += new System.EventHandler(this.button15_Click);
            // 
            // button16
            // 
            this.button16.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(192)))), ((int)(((byte)(192)))));
            this.button16.Location = new System.Drawing.Point(82, 150);
            this.button16.Name = "button16";
            this.button16.Size = new System.Drawing.Size(35, 23);
            this.button16.TabIndex = 31;
            this.button16.Text = "V";
            this.button16.UseVisualStyleBackColor = false;
            // 
            // button17
            // 
            this.button17.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(192)))), ((int)(((byte)(192)))));
            this.button17.Location = new System.Drawing.Point(29, 82);
            this.button17.Name = "button17";
            this.button17.Size = new System.Drawing.Size(35, 23);
            this.button17.TabIndex = 32;
            this.button17.Text = "TL";
            this.button17.UseVisualStyleBackColor = false;
            // 
            // button18
            // 
            this.button18.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(192)))), ((int)(((byte)(192)))));
            this.button18.Location = new System.Drawing.Point(20, 111);
            this.button18.Name = "button18";
            this.button18.Size = new System.Drawing.Size(35, 23);
            this.button18.TabIndex = 33;
            this.button18.Text = "<";
            this.button18.UseVisualStyleBackColor = false;
            // 
            // button19
            // 
            this.button19.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(192)))), ((int)(((byte)(192)))));
            this.button19.Location = new System.Drawing.Point(29, 140);
            this.button19.Name = "button19";
            this.button19.Size = new System.Drawing.Size(35, 23);
            this.button19.TabIndex = 34;
            this.button19.Text = "AL";
            this.button19.UseVisualStyleBackColor = false;
            // 
            // button20
            // 
            this.button20.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(192)))), ((int)(((byte)(192)))));
            this.button20.Location = new System.Drawing.Point(136, 82);
            this.button20.Name = "button20";
            this.button20.Size = new System.Drawing.Size(35, 23);
            this.button20.TabIndex = 35;
            this.button20.Text = "TR";
            this.button20.UseVisualStyleBackColor = false;
            // 
            // button21
            // 
            this.button21.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(192)))), ((int)(((byte)(192)))));
            this.button21.Location = new System.Drawing.Point(148, 111);
            this.button21.Name = "button21";
            this.button21.Size = new System.Drawing.Size(35, 23);
            this.button21.TabIndex = 36;
            this.button21.Text = ">";
            this.button21.UseVisualStyleBackColor = false;
            // 
            // button22
            // 
            this.button22.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(192)))), ((int)(((byte)(192)))));
            this.button22.Location = new System.Drawing.Point(136, 140);
            this.button22.Name = "button22";
            this.button22.Size = new System.Drawing.Size(35, 23);
            this.button22.TabIndex = 37;
            this.button22.Text = "AR";
            this.button22.UseVisualStyleBackColor = false;
            // 
            // button23
            // 
            this.button23.BackColor = System.Drawing.Color.Silver;
            this.button23.Location = new System.Drawing.Point(20, 28);
            this.button23.Name = "button23";
            this.button23.Size = new System.Drawing.Size(35, 23);
            this.button23.TabIndex = 38;
            this.button23.Text = "A";
            this.button23.UseVisualStyleBackColor = false;
            this.button23.Click += new System.EventHandler(this.button23_Click);
            // 
            // button24
            // 
            this.button24.BackColor = System.Drawing.Color.Silver;
            this.button24.Location = new System.Drawing.Point(148, 28);
            this.button24.Name = "button24";
            this.button24.Size = new System.Drawing.Size(35, 23);
            this.button24.TabIndex = 39;
            this.button24.Text = "B";
            this.button24.UseVisualStyleBackColor = false;
            this.button24.Click += new System.EventHandler(this.button24_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(79, 28);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(53, 13);
            this.label2.TabIndex = 40;
            this.label2.Text = "REMOTE";
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(79, 28);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(53, 13);
            this.label12.TabIndex = 40;
            this.label12.Text = "REMOTE";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.label12);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.button24);
            this.groupBox1.Controls.Add(this.button23);
            this.groupBox1.Controls.Add(this.button22);
            this.groupBox1.Controls.Add(this.button21);
            this.groupBox1.Controls.Add(this.button20);
            this.groupBox1.Controls.Add(this.button19);
            this.groupBox1.Controls.Add(this.button18);
            this.groupBox1.Controls.Add(this.button17);
            this.groupBox1.Controls.Add(this.button16);
            this.groupBox1.Controls.Add(this.button15);
            this.groupBox1.Controls.Add(this.button14);
            this.groupBox1.Controls.Add(this.button13);
            this.groupBox1.Controls.Add(this.button12);
            this.groupBox1.Controls.Add(this.button11);
            this.groupBox1.Controls.Add(this.button10);
            this.groupBox1.Controls.Add(this.button9);
            this.groupBox1.Controls.Add(this.button8);
            this.groupBox1.Controls.Add(this.button7);
            this.groupBox1.Controls.Add(this.button6);
            this.groupBox1.Controls.Add(this.button5);
            this.groupBox1.Controls.Add(this.button4);
            this.groupBox1.Controls.Add(this.button3);
            this.groupBox1.Controls.Add(this.button2);
            this.groupBox1.Location = new System.Drawing.Point(796, 71);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(204, 322);
            this.groupBox1.TabIndex = 41;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "groupBox1";
            this.groupBox1.Visible = false;
            // 
            // Basic_frm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(796, 451);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.helptext);
            this.Controls.Add(this.tabControl1);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "Basic_frm";
            this.Text = "Basic Compiler - ";
            this.SizeChanged += new System.EventHandler(this.Basic_frm_SizeChanged);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage1.PerformLayout();
            this.tabPage2.ResumeLayout(false);
            this.tabPage2.PerformLayout();
            this.tabPage3.ResumeLayout(false);
            this.tabPage3.PerformLayout();
            this.tabPage4.ResumeLayout(false);
            this.tabPage4.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        void input_TextChanged(object sender, System.EventArgs e)
        {
            editfl = true;
        }

        #endregion

        private System.Windows.Forms.TextBox output;
        private System.Windows.Forms.TextBox comPort;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem loadToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem compilerToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem compileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem downloadToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
        private System.Windows.Forms.TextBox fname;
        private System.Windows.Forms.ProgressBar progressBar1;
        private System.Windows.Forms.Timer timer1;
        private System.Windows.Forms.RichTextBox input;
        private System.Windows.Forms.ToolStripMenuItem simulatorToolStripMenuItem;
        private System.Windows.Forms.Label helptext;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.TabPage tabPage3;
        private System.Windows.Forms.TextBox term;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ToolStripMenuItem newToolStripMenuItem;
        private System.Windows.Forms.ListBox comselect;
        private System.Windows.Forms.ToolStripMenuItem saveAsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem extendToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem loadBinaryToolStripMenuItem;
        private System.Windows.Forms.TabPage tabPage4;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.CheckBox filtercb;
        private System.Windows.Forms.Button ldWeb;
        private System.Windows.Forms.Button ldFile;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ToolStripMenuItem imageToolStripMenuItem;
        private System.Windows.Forms.CheckBox checkBox1;
        private System.Windows.Forms.TextBox textBox2;
        private System.Windows.Forms.CheckBox checkBox2;
        private System.Windows.Forms.Button stopBtn;
        private System.Windows.Forms.Button listBtn;
        private System.Windows.Forms.Button runBtn;
        private System.Windows.Forms.ToolStripMenuItem reLoadToolStripMenuItem;
        private System.Windows.Forms.TextBox textBox3;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.Button button4;
        private System.Windows.Forms.Button button5;
        private System.Windows.Forms.Button button6;
        private System.Windows.Forms.Button button7;
        private System.Windows.Forms.Button button8;
        private System.Windows.Forms.Button button9;
        private System.Windows.Forms.Button button10;
        private System.Windows.Forms.Button button11;
        private System.Windows.Forms.Button button12;
        private System.Windows.Forms.Button button13;
        private System.Windows.Forms.Button button14;
        private System.Windows.Forms.Button button15;
        private System.Windows.Forms.Button button16;
        private System.Windows.Forms.Button button17;
        private System.Windows.Forms.Button button18;
        private System.Windows.Forms.Button button19;
        private System.Windows.Forms.Button button20;
        private System.Windows.Forms.Button button21;
        private System.Windows.Forms.Button button22;
        private System.Windows.Forms.Button button23;
        private System.Windows.Forms.Button button24;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.GroupBox groupBox1;
    }
}