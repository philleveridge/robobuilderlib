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
            this.input = new System.Windows.Forms.TextBox();
            this.close_btn = new System.Windows.Forms.Button();
            this.compile_btn = new System.Windows.Forms.Button();
            this.download_btn = new System.Windows.Forms.Button();
            this.output = new System.Windows.Forms.TextBox();
            this.load_btn = new System.Windows.Forms.Button();
            this.save_btn = new System.Windows.Forms.Button();
            this.fname = new System.Windows.Forms.Label();
            this.comPort = new System.Windows.Forms.TextBox();
            this.button1 = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // input
            // 
            this.input.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.input.Location = new System.Drawing.Point(12, 33);
            this.input.Multiline = true;
            this.input.Name = "input";
            this.input.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.input.Size = new System.Drawing.Size(523, 191);
            this.input.TabIndex = 0;
            this.input.Text = "\'      test program\r\n\'\r\n       PRINT \"Test Passive\"\r\nLoop:  SERVO 12=@ \'set servo" +
                " passive\r\n       Print \"Servo 12 = \"; $SERVO:12\r\n       wait 500   \'0.5secs\r\n   " +
                "    goto Loop\r\n";
            // 
            // close_btn
            // 
            this.close_btn.Location = new System.Drawing.Point(481, 308);
            this.close_btn.Name = "close_btn";
            this.close_btn.Size = new System.Drawing.Size(55, 27);
            this.close_btn.TabIndex = 1;
            this.close_btn.Text = "Close";
            this.close_btn.UseVisualStyleBackColor = true;
            this.close_btn.Click += new System.EventHandler(this.close_btn_Click);
            // 
            // compile_btn
            // 
            this.compile_btn.Location = new System.Drawing.Point(12, 306);
            this.compile_btn.Name = "compile_btn";
            this.compile_btn.Size = new System.Drawing.Size(55, 27);
            this.compile_btn.TabIndex = 2;
            this.compile_btn.Text = "Compile";
            this.compile_btn.UseVisualStyleBackColor = true;
            this.compile_btn.Click += new System.EventHandler(this.compile_btn_Click);
            // 
            // download_btn
            // 
            this.download_btn.Location = new System.Drawing.Point(84, 306);
            this.download_btn.Name = "download_btn";
            this.download_btn.Size = new System.Drawing.Size(74, 27);
            this.download_btn.TabIndex = 3;
            this.download_btn.Text = "Download";
            this.download_btn.UseVisualStyleBackColor = true;
            this.download_btn.Click += new System.EventHandler(this.download_btn_Click);
            // 
            // output
            // 
            this.output.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(192)))), ((int)(((byte)(128)))));
            this.output.Location = new System.Drawing.Point(12, 230);
            this.output.Multiline = true;
            this.output.Name = "output";
            this.output.ReadOnly = true;
            this.output.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.output.Size = new System.Drawing.Size(523, 65);
            this.output.TabIndex = 4;
            // 
            // load_btn
            // 
            this.load_btn.Location = new System.Drawing.Point(321, 308);
            this.load_btn.Name = "load_btn";
            this.load_btn.Size = new System.Drawing.Size(74, 27);
            this.load_btn.TabIndex = 5;
            this.load_btn.Text = "Load";
            this.load_btn.UseVisualStyleBackColor = true;
            this.load_btn.Click += new System.EventHandler(this.load_btn_Click);
            // 
            // save_btn
            // 
            this.save_btn.Location = new System.Drawing.Point(401, 308);
            this.save_btn.Name = "save_btn";
            this.save_btn.Size = new System.Drawing.Size(74, 27);
            this.save_btn.TabIndex = 6;
            this.save_btn.Text = "Save";
            this.save_btn.UseVisualStyleBackColor = true;
            this.save_btn.Click += new System.EventHandler(this.save_btn_Click);
            // 
            // fname
            // 
            this.fname.Location = new System.Drawing.Point(12, 9);
            this.fname.Name = "fname";
            this.fname.Size = new System.Drawing.Size(523, 21);
            this.fname.TabIndex = 7;
            // 
            // comPort
            // 
            this.comPort.Location = new System.Drawing.Point(18, 8);
            this.comPort.Name = "comPort";
            this.comPort.Size = new System.Drawing.Size(87, 20);
            this.comPort.TabIndex = 9;
            this.comPort.Text = "COM5";
            // 
            // button1
            // 
            this.button1.BackColor = System.Drawing.Color.Red;
            this.button1.Location = new System.Drawing.Point(111, 8);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(36, 21);
            this.button1.TabIndex = 10;
            this.button1.Text = "Con";
            this.button1.UseVisualStyleBackColor = false;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // Basic_frm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(549, 342);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.comPort);
            this.Controls.Add(this.fname);
            this.Controls.Add(this.save_btn);
            this.Controls.Add(this.load_btn);
            this.Controls.Add(this.output);
            this.Controls.Add(this.download_btn);
            this.Controls.Add(this.compile_btn);
            this.Controls.Add(this.close_btn);
            this.Controls.Add(this.input);
            this.Name = "Basic_frm";
            this.Text = "Basic Compiler V0.1 ";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox input;
        private System.Windows.Forms.Button close_btn;
        private System.Windows.Forms.Button compile_btn;
        private System.Windows.Forms.Button download_btn;
        private System.Windows.Forms.TextBox output;
        private System.Windows.Forms.Button load_btn;
        private System.Windows.Forms.Button save_btn;
        private System.Windows.Forms.Label fname;
        private System.Windows.Forms.TextBox comPort;
        private System.Windows.Forms.Button button1;
    }
}