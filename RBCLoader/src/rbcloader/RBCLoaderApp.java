/*
 * RBCLoaderApp.java
 */

package rbcloader;

import org.jdesktop.application.Action;
import org.jdesktop.application.Application;
import org.jdesktop.application.SingleFrameApplication;

/**
 * The main class of the application.
 */
public class RBCLoaderApp extends SingleFrameApplication {

    /**
     * At startup create and show the main frame of the application.
     */
    RBCLoaderView mform;

    @Override protected void startup() {
        show((mform = new RBCLoaderView(this)));
    }

    /**
     * This method is to initialize the specified window by injecting resources.
     * Windows shown in our application come fully initialized from the GUI
     * builder, so this additional configuration is not needed.
     */
    @Override protected void configureWindow(java.awt.Window root) {
    }

    /**
     * A convenient static getter for the application instance.
     * @return the instance of RBCLoaderApp
     */
    public static RBCLoaderApp getApplication() {
        return Application.getInstance(RBCLoaderApp.class);
    }

    /**
     * Main method launching the application.
     */
    public static void main(String[] args) {
        launch(RBCLoaderApp.class, args);
    }

    @Action
    public void Download()
    {
        String portName = mform.jTextField1.getText();
        int baud = Integer.parseInt(mform.jTextField2.getText());
        String fileName = mform.jTextField3.getText();

        Rloader r = new Rloader(portName, baud);
        r.download(fileName);
        r.close();

        System.exit(0); // dome
    }
}
