package wesnoth_eclipse_plugin.product;

import org.eclipse.ui.application.ActionBarAdvisor;
import org.eclipse.ui.application.IActionBarConfigurer;
import org.eclipse.ui.application.IWorkbenchWindowConfigurer;
import org.eclipse.ui.application.WorkbenchWindowAdvisor;

public class ApplicationWorkbenchWindowAdvisor extends WorkbenchWindowAdvisor {

    public ApplicationWorkbenchWindowAdvisor(IWorkbenchWindowConfigurer configurer) {
        super(configurer);
    }

    @Override
	public ActionBarAdvisor createActionBarAdvisor(IActionBarConfigurer configurer) {
        return new ApplicationActionBarAdvisor(configurer);
    }

    @Override
	public void preWindowOpen() {
        IWorkbenchWindowConfigurer configurer = getWindowConfigurer();
        configurer.setShowMenuBar(true);
        configurer.setShowProgressIndicator(true);
        configurer.setShowStatusLine(true);
        configurer.setShowPerspectiveBar(true);
        configurer.setShowFastViewBars(true);
//        configurer.setInitialSize(new Point(400, 300));
//        configurer.setShowCoolBar(false);
//        configurer.setShowStatusLine(false);
    }
}
