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
        private int ID;

        public int val { get { return servo.Value; } set { servo.Value = value; } }
        public int id { get { return ID; } set { label1.Text = "ID: " + value; ID = value; } }
        public int  io  { get { return (red.Checked ? 2 : 0) + (blue.Checked ? 1 : 0); } set { red.Checked = ((value & 2) == 2); blue.Checked = ((value & 1) == 1); } }
        public bool pm  { get { return passive.Checked;} set { passive.Checked = value; } }

        public servoUC()
        {
            InitializeComponent();
            io = 0;
            val = 0;
            id = 0;
        }

        private void servo_Scroll(object sender, ScrollEventArgs e)
        {
            label2.Text = "Val: " + servo.Value;
        }

        void servo_ValueChanged(object sender, System.EventArgs e)
        {
            label2.Text = "Val: " + servo.Value;
        }

        private void passive_CheckedChanged(object sender, EventArgs e)
        {

        }
    }
}
