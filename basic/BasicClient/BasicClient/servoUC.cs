using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace RobobuilderLib
{
    public partial class servoUC : UserControl
    {
        public int val { get { return servo.Value; } set { servo.Value = value; } }
        public int id { set { label1.Text = "ID: " + value; } }

        public int io { set { red.Checked = ((value & 2) == 2); blue.Checked = ((value & 1) == 1); } }

        public bool pm { set { passive.Enabled = value; } }

        public servoUC()
        {
            InitializeComponent();
            io = 0;
            val = 0;
            pm = true;
        }

        private void servo_Scroll(object sender, ScrollEventArgs e)
        {
            label2.Text = "Val: " + servo.Value;
        }

        void servo_ValueChanged(object sender, System.EventArgs e)
        {
            label2.Text = "Val: " + servo.Value;
        }
    }
}
