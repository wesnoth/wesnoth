package wesnoth_eclipse_plugin.wizards;

import org.eclipse.core.runtime.IConfigurationElement;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.IWizardPage;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;

public class CampaignNewWizard extends Wizard implements INewWizard {
	
	private org.eclipse.ui.IWorkbench workbench;
	private org.eclipse.jface.viewers.IStructuredSelection selection;
	
	protected NewCampaignCreationPage fMainPage;
	private IConfigurationElement fConfig;
	
	public void addPages() {
		fMainPage = new NewCampaignCreationPage("blah");
		addPage(fMainPage);
	}
	
//    protected PluginContentPage fContentPage;

	public CampaignNewWizard() {
//	    setDefaultPageImageDescriptor(PDEPluginImages.DESC_NEWPPRJ_WIZ);
  //      setDialogSettings(PDEPlugin.getDefault().getDialogSettings());
    //    setWindowTitle(PDEUIMessages.NewProjectWizard_title);
	//	System.exit(0);
		setWindowTitle("Create a new Campaign");
        setNeedsProgressMonitor(true);
        
      //  fPluginData = new PluginFieldData();
	}

    public CampaignNewWizard(String osgiFramework) {
        this();
     //   fPluginData.setOSGiFramework(osgiFramework);
}

	
	
	@Override
	public void init(IWorkbench workbench, IStructuredSelection selection) {
		this.workbench = workbench;
		this.selection = selection;
	}
	
	
//	public void init(IWorkbench workbench, IStructuredSelection selection) {
//		// TODO Auto-generated method stub	
//	}

	@Override
	public boolean performFinish() {
		
//	     try {
//             fMainPage.updateData();
////             fContentPage.updateData();
//  //           IDialogSettings settings = getDialogSettings();
////             if (settings != null) {
////                     fMainPage.saveSettings(settings);
////                     fContentPage.saveSettings(settings);
////             }
//             BasicNewProjectResourceWizard.updatePerspective(fConfig);
//       //      IPluginContentWizard contentWizard = fWizardListPage.getSelectedWizard();
//       //      getContainer().run(false, true, new NewProjectCreationOperation(fPluginData, fProjectProvider, contentWizard));
//
//       //      IWorkingSet[] workingSets = fMainPage.getSelectedWorkingSets();
////             if (workingSets.length > 0)
////                     getWorkbench().getWorkingSetManager().addToWorkingSets(fProjectProvider.getProject(), workingSets);
//
//             return true;
////     } catch (InvocationTargetException e) {
//// //            PDEPlugin.logException(e);
////     } catch (InterruptedException e) {
////     }
////     return false;
             return true;
	     }
	
    /* (non-Javadoc)
     * @see org.eclipse.jface.wizard.Wizard#canFinish()
     */
    public boolean canFinish() {
            IWizardPage page = getContainer().getCurrentPage();
            return super.canFinish() && page != fMainPage;
    }

	
}
