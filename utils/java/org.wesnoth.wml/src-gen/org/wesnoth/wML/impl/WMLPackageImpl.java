/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML.impl;

import org.eclipse.emf.ecore.EAttribute;
import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EPackage;
import org.eclipse.emf.ecore.EReference;

import org.eclipse.emf.ecore.impl.EPackageImpl;

import org.wesnoth.wML.WMLFactory;
import org.wesnoth.wML.WMLKey;
import org.wesnoth.wML.WMLKeyValue;
import org.wesnoth.wML.WMLMacro;
import org.wesnoth.wML.WMLPackage;
import org.wesnoth.wML.WMLRoot;
import org.wesnoth.wML.WMLTag;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model <b>Package</b>.
 * <!-- end-user-doc -->
 * @generated
 */
public class WMLPackageImpl extends EPackageImpl implements WMLPackage
{
  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlRootEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlMacroEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlTagEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlKeyEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlKeyValueEClass = null;

  /**
   * Creates an instance of the model <b>Package</b>, registered with
   * {@link org.eclipse.emf.ecore.EPackage.Registry EPackage.Registry} by the package
   * package URI value.
   * <p>Note: the correct way to create the package is via the static
   * factory method {@link #init init()}, which also performs
   * initialization of the package, or returns the registered package,
   * if one already exists.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.eclipse.emf.ecore.EPackage.Registry
   * @see org.wesnoth.wML.WMLPackage#eNS_URI
   * @see #init()
   * @generated
   */
  private WMLPackageImpl()
  {
    super(eNS_URI, WMLFactory.eINSTANCE);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private static boolean isInited = false;

  /**
   * Creates, registers, and initializes the <b>Package</b> for this model, and for any others upon which it depends.
   * 
   * <p>This method is used to initialize {@link WMLPackage#eINSTANCE} when that field is accessed.
   * Clients should not invoke it directly. Instead, they should simply access that field to obtain the package.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #eNS_URI
   * @see #createPackageContents()
   * @see #initializePackageContents()
   * @generated
   */
  public static WMLPackage init()
  {
    if (isInited) return (WMLPackage)EPackage.Registry.INSTANCE.getEPackage(WMLPackage.eNS_URI);

    // Obtain or create and register package
    WMLPackageImpl theWMLPackage = (WMLPackageImpl)(EPackage.Registry.INSTANCE.get(eNS_URI) instanceof WMLPackageImpl ? EPackage.Registry.INSTANCE.get(eNS_URI) : new WMLPackageImpl());

    isInited = true;

    // Create package meta-data objects
    theWMLPackage.createPackageContents();

    // Initialize created meta-data
    theWMLPackage.initializePackageContents();

    // Mark meta-data to indicate it can't be changed
    theWMLPackage.freeze();

  
    // Update the registry and return the package
    EPackage.Registry.INSTANCE.put(WMLPackage.eNS_URI, theWMLPackage);
    return theWMLPackage;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLRoot()
  {
    return wmlRootEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLRoot_Rtags()
  {
    return (EReference)wmlRootEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLRoot_Rmacros()
  {
    return (EReference)wmlRootEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLMacro()
  {
    return wmlMacroEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLMacro_Name()
  {
    return (EAttribute)wmlMacroEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLMacro_Value()
  {
    return (EAttribute)wmlMacroEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLTag()
  {
    return wmlTagEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLTag_Name()
  {
    return (EAttribute)wmlTagEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLTag_Ttags()
  {
    return (EReference)wmlTagEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLTag_Tkeys()
  {
    return (EReference)wmlTagEClass.getEStructuralFeatures().get(2);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLTag_Tmacros()
  {
    return (EReference)wmlTagEClass.getEStructuralFeatures().get(3);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLTag_EndName()
  {
    return (EAttribute)wmlTagEClass.getEStructuralFeatures().get(4);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLKey()
  {
    return wmlKeyEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLKey_KeyName()
  {
    return (EAttribute)wmlKeyEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLKey_Value()
  {
    return (EReference)wmlKeyEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLKeyValue()
  {
    return wmlKeyValueEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLKeyValue_Key1Value()
  {
    return (EAttribute)wmlKeyValueEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLKeyValue_Key2Value()
  {
    return (EReference)wmlKeyValueEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLFactory getWMLFactory()
  {
    return (WMLFactory)getEFactoryInstance();
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private boolean isCreated = false;

  /**
   * Creates the meta-model objects for the package.  This method is
   * guarded to have no affect on any invocation but its first.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void createPackageContents()
  {
    if (isCreated) return;
    isCreated = true;

    // Create classes and their features
    wmlRootEClass = createEClass(WML_ROOT);
    createEReference(wmlRootEClass, WML_ROOT__RTAGS);
    createEReference(wmlRootEClass, WML_ROOT__RMACROS);

    wmlMacroEClass = createEClass(WML_MACRO);
    createEAttribute(wmlMacroEClass, WML_MACRO__NAME);
    createEAttribute(wmlMacroEClass, WML_MACRO__VALUE);

    wmlTagEClass = createEClass(WML_TAG);
    createEAttribute(wmlTagEClass, WML_TAG__NAME);
    createEReference(wmlTagEClass, WML_TAG__TTAGS);
    createEReference(wmlTagEClass, WML_TAG__TKEYS);
    createEReference(wmlTagEClass, WML_TAG__TMACROS);
    createEAttribute(wmlTagEClass, WML_TAG__END_NAME);

    wmlKeyEClass = createEClass(WML_KEY);
    createEAttribute(wmlKeyEClass, WML_KEY__KEY_NAME);
    createEReference(wmlKeyEClass, WML_KEY__VALUE);

    wmlKeyValueEClass = createEClass(WML_KEY_VALUE);
    createEAttribute(wmlKeyValueEClass, WML_KEY_VALUE__KEY1_VALUE);
    createEReference(wmlKeyValueEClass, WML_KEY_VALUE__KEY2_VALUE);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private boolean isInitialized = false;

  /**
   * Complete the initialization of the package and its meta-model.  This
   * method is guarded to have no affect on any invocation but its first.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void initializePackageContents()
  {
    if (isInitialized) return;
    isInitialized = true;

    // Initialize package
    setName(eNAME);
    setNsPrefix(eNS_PREFIX);
    setNsURI(eNS_URI);

    // Create type parameters

    // Set bounds for type parameters

    // Add supertypes to classes

    // Initialize classes and features; add operations and parameters
    initEClass(wmlRootEClass, WMLRoot.class, "WMLRoot", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEReference(getWMLRoot_Rtags(), this.getWMLTag(), null, "Rtags", null, 0, -1, WMLRoot.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLRoot_Rmacros(), this.getWMLMacro(), null, "Rmacros", null, 0, -1, WMLRoot.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlMacroEClass, WMLMacro.class, "WMLMacro", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLMacro_Name(), ecorePackage.getEString(), "name", null, 0, 1, WMLMacro.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLMacro_Value(), ecorePackage.getEString(), "value", null, 0, -1, WMLMacro.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, !IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlTagEClass, WMLTag.class, "WMLTag", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLTag_Name(), ecorePackage.getEString(), "name", null, 0, 1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLTag_Ttags(), this.getWMLTag(), null, "Ttags", null, 0, -1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLTag_Tkeys(), this.getWMLKey(), null, "Tkeys", null, 0, -1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLTag_Tmacros(), this.getWMLMacro(), null, "Tmacros", null, 0, -1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLTag_EndName(), ecorePackage.getEString(), "endName", null, 0, 1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlKeyEClass, WMLKey.class, "WMLKey", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLKey_KeyName(), ecorePackage.getEString(), "keyName", null, 0, 1, WMLKey.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLKey_Value(), this.getWMLKeyValue(), null, "value", null, 0, 1, WMLKey.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlKeyValueEClass, WMLKeyValue.class, "WMLKeyValue", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLKeyValue_Key1Value(), ecorePackage.getEString(), "key1Value", null, 0, 1, WMLKeyValue.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLKeyValue_Key2Value(), this.getWMLMacro(), null, "key2Value", null, 0, 1, WMLKeyValue.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    // Create resource
    createResource(eNS_URI);
  }

} //WMLPackageImpl
