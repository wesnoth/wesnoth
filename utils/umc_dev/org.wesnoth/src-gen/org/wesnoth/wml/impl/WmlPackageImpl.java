/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml.impl;

import java.io.Serializable;

import org.eclipse.emf.ecore.EAttribute;
import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EGenericType;
import org.eclipse.emf.ecore.EOperation;
import org.eclipse.emf.ecore.EPackage;
import org.eclipse.emf.ecore.EReference;

import org.eclipse.emf.ecore.impl.EPackageImpl;

import org.wesnoth.wml.MacroTokens;
import org.wesnoth.wml.WMLArrayCall;
import org.wesnoth.wml.WMLExpression;
import org.wesnoth.wml.WMLGrammarElement;
import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLKeyValue;
import org.wesnoth.wml.WMLLuaCode;
import org.wesnoth.wml.WMLMacroCall;
import org.wesnoth.wml.WMLMacroCallParameter;
import org.wesnoth.wml.WMLMacroDefine;
import org.wesnoth.wml.WMLMacroParameter;
import org.wesnoth.wml.WMLPreprocIF;
import org.wesnoth.wml.WMLRoot;
import org.wesnoth.wml.WMLRootExpression;
import org.wesnoth.wml.WMLTag;
import org.wesnoth.wml.WMLTextdomain;
import org.wesnoth.wml.WMLValue;
import org.wesnoth.wml.WMLValuedExpression;
import org.wesnoth.wml.WmlFactory;
import org.wesnoth.wml.WmlPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model <b>Package</b>.
 * <!-- end-user-doc -->
 * @generated
 */
public class WmlPackageImpl extends EPackageImpl implements WmlPackage
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
  private EClass wmlGrammarElementEClass = null;

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
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlMacroCallEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlMacroCallParameterEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlArrayCallEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlMacroDefineEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlPreprocIFEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlRootExpressionEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlExpressionEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlValuedExpressionEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlTextdomainEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlLuaCodeEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlMacroParameterEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass wmlValueEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass macroTokensEClass = null;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private EClass eSerializableEClass = null;

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
   * @see org.wesnoth.wml.WmlPackage#eNS_URI
   * @see #init()
   * @generated
   */
  private WmlPackageImpl()
  {
    super(eNS_URI, WmlFactory.eINSTANCE);
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
   * <p>This method is used to initialize {@link WmlPackage#eINSTANCE} when that field is accessed.
   * Clients should not invoke it directly. Instead, they should simply access that field to obtain the package.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #eNS_URI
   * @see #createPackageContents()
   * @see #initializePackageContents()
   * @generated
   */
  public static WmlPackage init()
  {
    if (isInited) return (WmlPackage)EPackage.Registry.INSTANCE.getEPackage(WmlPackage.eNS_URI);

    // Obtain or create and register package
    WmlPackageImpl theWmlPackage = (WmlPackageImpl)(EPackage.Registry.INSTANCE.get(eNS_URI) instanceof WmlPackageImpl ? EPackage.Registry.INSTANCE.get(eNS_URI) : new WmlPackageImpl());

    isInited = true;

    // Create package meta-data objects
    theWmlPackage.createPackageContents();

    // Initialize created meta-data
    theWmlPackage.initializePackageContents();

    // Mark meta-data to indicate it can't be changed
    theWmlPackage.freeze();

  
    // Update the registry and return the package
    EPackage.Registry.INSTANCE.put(WmlPackage.eNS_URI, theWmlPackage);
    return theWmlPackage;
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
  public EReference getWMLRoot_Expressions()
  {
    return (EReference)wmlRootEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLGrammarElement()
  {
    return wmlGrammarElementEClass;
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
  public EAttribute getWMLTag_Plus()
  {
    return (EAttribute)wmlTagEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLTag_Expressions()
  {
    return (EReference)wmlTagEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLTag_EndName()
  {
    return (EAttribute)wmlTagEClass.getEStructuralFeatures().get(2);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLTag__InhertedTagName()
  {
    return (EAttribute)wmlTagEClass.getEStructuralFeatures().get(3);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLTag__NeedingExpansion()
  {
    return (EAttribute)wmlTagEClass.getEStructuralFeatures().get(4);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLTag__Description()
  {
    return (EAttribute)wmlTagEClass.getEStructuralFeatures().get(5);
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
  public EReference getWMLKey_Values()
  {
    return (EReference)wmlKeyEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLKey_Eol()
  {
    return (EAttribute)wmlKeyEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLKey__Enum()
  {
    return (EAttribute)wmlKeyEClass.getEStructuralFeatures().get(2);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLKey__Translatable()
  {
    return (EAttribute)wmlKeyEClass.getEStructuralFeatures().get(3);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLKey__DataType()
  {
    return (EAttribute)wmlKeyEClass.getEStructuralFeatures().get(4);
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
  public EClass getWMLMacroCall()
  {
    return wmlMacroCallEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLMacroCall_Point()
  {
    return (EAttribute)wmlMacroCallEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLMacroCall_Relative()
  {
    return (EAttribute)wmlMacroCallEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLMacroCall_Parameters()
  {
    return (EReference)wmlMacroCallEClass.getEStructuralFeatures().get(2);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLMacroCallParameter()
  {
    return wmlMacroCallParameterEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLArrayCall()
  {
    return wmlArrayCallEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLArrayCall_Value()
  {
    return (EReference)wmlArrayCallEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLMacroDefine()
  {
    return wmlMacroDefineEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLMacroDefine_Expressions()
  {
    return (EReference)wmlMacroDefineEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLMacroDefine_EndName()
  {
    return (EAttribute)wmlMacroDefineEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLPreprocIF()
  {
    return wmlPreprocIFEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLPreprocIF_Expressions()
  {
    return (EReference)wmlPreprocIFEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLPreprocIF_Elses()
  {
    return (EAttribute)wmlPreprocIFEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EReference getWMLPreprocIF_ElseExpressions()
  {
    return (EReference)wmlPreprocIFEClass.getEStructuralFeatures().get(2);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLPreprocIF_EndName()
  {
    return (EAttribute)wmlPreprocIFEClass.getEStructuralFeatures().get(3);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLRootExpression()
  {
    return wmlRootExpressionEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLExpression()
  {
    return wmlExpressionEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLExpression_Name()
  {
    return (EAttribute)wmlExpressionEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLExpression__LuaBased()
  {
    return (EAttribute)wmlExpressionEClass.getEStructuralFeatures().get(1);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLExpression__DefinitionLocation()
  {
    return (EAttribute)wmlExpressionEClass.getEStructuralFeatures().get(2);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLExpression__DefinitionOffset()
  {
    return (EAttribute)wmlExpressionEClass.getEStructuralFeatures().get(3);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLExpression__Cardinality()
  {
    return (EAttribute)wmlExpressionEClass.getEStructuralFeatures().get(4);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLValuedExpression()
  {
    return wmlValuedExpressionEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLTextdomain()
  {
    return wmlTextdomainEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLLuaCode()
  {
    return wmlLuaCodeEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLLuaCode_Value()
  {
    return (EAttribute)wmlLuaCodeEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLMacroParameter()
  {
    return wmlMacroParameterEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EAttribute getWMLMacroParameter_Value()
  {
    return (EAttribute)wmlMacroParameterEClass.getEStructuralFeatures().get(0);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getWMLValue()
  {
    return wmlValueEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getMacroTokens()
  {
    return macroTokensEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EClass getESerializable()
  {
    return eSerializableEClass;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WmlFactory getWmlFactory()
  {
    return (WmlFactory)getEFactoryInstance();
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
    createEReference(wmlRootEClass, WML_ROOT__EXPRESSIONS);

    wmlGrammarElementEClass = createEClass(WML_GRAMMAR_ELEMENT);

    wmlTagEClass = createEClass(WML_TAG);
    createEAttribute(wmlTagEClass, WML_TAG__PLUS);
    createEReference(wmlTagEClass, WML_TAG__EXPRESSIONS);
    createEAttribute(wmlTagEClass, WML_TAG__END_NAME);
    createEAttribute(wmlTagEClass, WML_TAG__INHERTED_TAG_NAME);
    createEAttribute(wmlTagEClass, WML_TAG__NEEDING_EXPANSION);
    createEAttribute(wmlTagEClass, WML_TAG__DESCRIPTION);

    wmlKeyEClass = createEClass(WML_KEY);
    createEReference(wmlKeyEClass, WML_KEY__VALUES);
    createEAttribute(wmlKeyEClass, WML_KEY__EOL);
    createEAttribute(wmlKeyEClass, WML_KEY__ENUM);
    createEAttribute(wmlKeyEClass, WML_KEY__TRANSLATABLE);
    createEAttribute(wmlKeyEClass, WML_KEY__DATA_TYPE);

    wmlKeyValueEClass = createEClass(WML_KEY_VALUE);

    wmlMacroCallEClass = createEClass(WML_MACRO_CALL);
    createEAttribute(wmlMacroCallEClass, WML_MACRO_CALL__POINT);
    createEAttribute(wmlMacroCallEClass, WML_MACRO_CALL__RELATIVE);
    createEReference(wmlMacroCallEClass, WML_MACRO_CALL__PARAMETERS);

    wmlMacroCallParameterEClass = createEClass(WML_MACRO_CALL_PARAMETER);

    wmlArrayCallEClass = createEClass(WML_ARRAY_CALL);
    createEReference(wmlArrayCallEClass, WML_ARRAY_CALL__VALUE);

    wmlMacroDefineEClass = createEClass(WML_MACRO_DEFINE);
    createEReference(wmlMacroDefineEClass, WML_MACRO_DEFINE__EXPRESSIONS);
    createEAttribute(wmlMacroDefineEClass, WML_MACRO_DEFINE__END_NAME);

    wmlPreprocIFEClass = createEClass(WML_PREPROC_IF);
    createEReference(wmlPreprocIFEClass, WML_PREPROC_IF__EXPRESSIONS);
    createEAttribute(wmlPreprocIFEClass, WML_PREPROC_IF__ELSES);
    createEReference(wmlPreprocIFEClass, WML_PREPROC_IF__ELSE_EXPRESSIONS);
    createEAttribute(wmlPreprocIFEClass, WML_PREPROC_IF__END_NAME);

    wmlRootExpressionEClass = createEClass(WML_ROOT_EXPRESSION);

    wmlExpressionEClass = createEClass(WML_EXPRESSION);
    createEAttribute(wmlExpressionEClass, WML_EXPRESSION__NAME);
    createEAttribute(wmlExpressionEClass, WML_EXPRESSION__LUA_BASED);
    createEAttribute(wmlExpressionEClass, WML_EXPRESSION__DEFINITION_LOCATION);
    createEAttribute(wmlExpressionEClass, WML_EXPRESSION__DEFINITION_OFFSET);
    createEAttribute(wmlExpressionEClass, WML_EXPRESSION__CARDINALITY);

    wmlValuedExpressionEClass = createEClass(WML_VALUED_EXPRESSION);

    wmlTextdomainEClass = createEClass(WML_TEXTDOMAIN);

    wmlLuaCodeEClass = createEClass(WML_LUA_CODE);
    createEAttribute(wmlLuaCodeEClass, WML_LUA_CODE__VALUE);

    wmlMacroParameterEClass = createEClass(WML_MACRO_PARAMETER);
    createEAttribute(wmlMacroParameterEClass, WML_MACRO_PARAMETER__VALUE);

    wmlValueEClass = createEClass(WML_VALUE);

    macroTokensEClass = createEClass(MACRO_TOKENS);

    eSerializableEClass = createEClass(ESERIALIZABLE);
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
    wmlRootEClass.getESuperTypes().add(this.getWMLGrammarElement());
    wmlGrammarElementEClass.getESuperTypes().add(this.getESerializable());
    wmlTagEClass.getESuperTypes().add(this.getWMLRootExpression());
    wmlKeyEClass.getESuperTypes().add(this.getWMLExpression());
    wmlKeyValueEClass.getESuperTypes().add(this.getWMLGrammarElement());
    wmlMacroCallEClass.getESuperTypes().add(this.getWMLKeyValue());
    wmlMacroCallEClass.getESuperTypes().add(this.getWMLMacroCallParameter());
    wmlMacroCallEClass.getESuperTypes().add(this.getWMLRootExpression());
    wmlMacroCallParameterEClass.getESuperTypes().add(this.getWMLGrammarElement());
    wmlArrayCallEClass.getESuperTypes().add(this.getWMLKeyValue());
    wmlMacroDefineEClass.getESuperTypes().add(this.getWMLRootExpression());
    wmlPreprocIFEClass.getESuperTypes().add(this.getWMLRootExpression());
    wmlRootExpressionEClass.getESuperTypes().add(this.getWMLExpression());
    wmlExpressionEClass.getESuperTypes().add(this.getWMLValuedExpression());
    wmlValuedExpressionEClass.getESuperTypes().add(this.getWMLGrammarElement());
    wmlTextdomainEClass.getESuperTypes().add(this.getWMLRootExpression());
    wmlLuaCodeEClass.getESuperTypes().add(this.getWMLKeyValue());
    wmlMacroParameterEClass.getESuperTypes().add(this.getWMLMacroCallParameter());
    wmlValueEClass.getESuperTypes().add(this.getWMLKeyValue());
    wmlValueEClass.getESuperTypes().add(this.getWMLValuedExpression());
    wmlValueEClass.getESuperTypes().add(this.getWMLMacroParameter());
    macroTokensEClass.getESuperTypes().add(this.getWMLMacroParameter());

    // Initialize classes and features; add operations and parameters
    initEClass(wmlRootEClass, WMLRoot.class, "WMLRoot", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEReference(getWMLRoot_Expressions(), this.getWMLRootExpression(), null, "Expressions", null, 0, -1, WMLRoot.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlGrammarElementEClass, WMLGrammarElement.class, "WMLGrammarElement", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);

    initEClass(wmlTagEClass, WMLTag.class, "WMLTag", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLTag_Plus(), ecorePackage.getEString(), "plus", "", 0, 1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLTag_Expressions(), this.getWMLExpression(), null, "Expressions", null, 0, -1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLTag_EndName(), ecorePackage.getEString(), "endName", "", 0, 1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLTag__InhertedTagName(), ecorePackage.getEString(), "_InhertedTagName", "", 0, 1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLTag__NeedingExpansion(), ecorePackage.getEBoolean(), "_NeedingExpansion", "false", 0, 1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLTag__Description(), ecorePackage.getEString(), "_Description", "", 0, 1, WMLTag.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    EOperation op = addEOperation(wmlTagEClass, null, "getWMLTags", 0, 1, IS_UNIQUE, IS_ORDERED);
    EGenericType g1 = createEGenericType(ecorePackage.getEEList());
    EGenericType g2 = createEGenericType(this.getWMLTag());
    g1.getETypeArguments().add(g2);
    initEOperation(op, g1);

    op = addEOperation(wmlTagEClass, null, "getWMLKeys", 0, 1, IS_UNIQUE, IS_ORDERED);
    g1 = createEGenericType(ecorePackage.getEEList());
    g2 = createEGenericType(this.getWMLKey());
    g1.getETypeArguments().add(g2);
    initEOperation(op, g1);

    initEClass(wmlKeyEClass, WMLKey.class, "WMLKey", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEReference(getWMLKey_Values(), this.getWMLKeyValue(), null, "values", null, 0, -1, WMLKey.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLKey_Eol(), ecorePackage.getEString(), "eol", "", 0, -1, WMLKey.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, !IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLKey__Enum(), ecorePackage.getEBoolean(), "_Enum", "false", 0, 1, WMLKey.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLKey__Translatable(), ecorePackage.getEBoolean(), "_Translatable", "false", 0, 1, WMLKey.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLKey__DataType(), ecorePackage.getEString(), "_DataType", "", 0, 1, WMLKey.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    addEOperation(wmlKeyEClass, ecorePackage.getEString(), "getValue", 0, 1, IS_UNIQUE, IS_ORDERED);

    initEClass(wmlKeyValueEClass, WMLKeyValue.class, "WMLKeyValue", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);

    initEClass(wmlMacroCallEClass, WMLMacroCall.class, "WMLMacroCall", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLMacroCall_Point(), ecorePackage.getEString(), "point", "", 0, 1, WMLMacroCall.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLMacroCall_Relative(), ecorePackage.getEString(), "relative", "", 0, 1, WMLMacroCall.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLMacroCall_Parameters(), this.getWMLMacroCallParameter(), null, "Parameters", null, 0, -1, WMLMacroCall.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlMacroCallParameterEClass, WMLMacroCallParameter.class, "WMLMacroCallParameter", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);

    initEClass(wmlArrayCallEClass, WMLArrayCall.class, "WMLArrayCall", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEReference(getWMLArrayCall_Value(), this.getWMLValue(), null, "value", null, 0, -1, WMLArrayCall.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlMacroDefineEClass, WMLMacroDefine.class, "WMLMacroDefine", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEReference(getWMLMacroDefine_Expressions(), this.getWMLValuedExpression(), null, "Expressions", null, 0, -1, WMLMacroDefine.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLMacroDefine_EndName(), ecorePackage.getEString(), "endName", "", 0, 1, WMLMacroDefine.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlPreprocIFEClass, WMLPreprocIF.class, "WMLPreprocIF", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEReference(getWMLPreprocIF_Expressions(), this.getWMLValuedExpression(), null, "Expressions", null, 0, -1, WMLPreprocIF.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLPreprocIF_Elses(), ecorePackage.getEString(), "Elses", "", 0, 1, WMLPreprocIF.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEReference(getWMLPreprocIF_ElseExpressions(), this.getWMLValuedExpression(), null, "ElseExpressions", null, 0, -1, WMLPreprocIF.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, IS_COMPOSITE, !IS_RESOLVE_PROXIES, !IS_UNSETTABLE, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLPreprocIF_EndName(), ecorePackage.getEString(), "endName", "", 0, 1, WMLPreprocIF.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlRootExpressionEClass, WMLRootExpression.class, "WMLRootExpression", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);

    initEClass(wmlExpressionEClass, WMLExpression.class, "WMLExpression", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLExpression_Name(), ecorePackage.getEString(), "name", "", 0, 1, WMLExpression.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLExpression__LuaBased(), ecorePackage.getEBoolean(), "_LuaBased", "false", 0, 1, WMLExpression.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLExpression__DefinitionLocation(), ecorePackage.getEString(), "_DefinitionLocation", "", 0, 1, WMLExpression.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLExpression__DefinitionOffset(), ecorePackage.getEInt(), "_DefinitionOffset", "0", 0, 1, WMLExpression.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);
    initEAttribute(getWMLExpression__Cardinality(), ecorePackage.getEChar(), "_Cardinality", " ", 0, 1, WMLExpression.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    addEOperation(wmlExpressionEClass, ecorePackage.getEBoolean(), "is_Required", 0, 1, IS_UNIQUE, IS_ORDERED);

    addEOperation(wmlExpressionEClass, ecorePackage.getEBoolean(), "is_Forbidden", 0, 1, IS_UNIQUE, IS_ORDERED);

    addEOperation(wmlExpressionEClass, ecorePackage.getEBoolean(), "is_Optional", 0, 1, IS_UNIQUE, IS_ORDERED);

    addEOperation(wmlExpressionEClass, ecorePackage.getEBoolean(), "is_Repeatable", 0, 1, IS_UNIQUE, IS_ORDERED);

    addEOperation(wmlExpressionEClass, ecorePackage.getEInt(), "getAllowedCount", 0, 1, IS_UNIQUE, IS_ORDERED);

    addEOperation(wmlExpressionEClass, ecorePackage.getEBoolean(), "isWMLTag", 0, 1, IS_UNIQUE, IS_ORDERED);

    addEOperation(wmlExpressionEClass, this.getWMLTag(), "asWMLTag", 0, 1, IS_UNIQUE, IS_ORDERED);

    addEOperation(wmlExpressionEClass, ecorePackage.getEBoolean(), "isWMLKey", 0, 1, IS_UNIQUE, IS_ORDERED);

    addEOperation(wmlExpressionEClass, this.getWMLKey(), "asWMLKey", 0, 1, IS_UNIQUE, IS_ORDERED);

    initEClass(wmlValuedExpressionEClass, WMLValuedExpression.class, "WMLValuedExpression", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);

    initEClass(wmlTextdomainEClass, WMLTextdomain.class, "WMLTextdomain", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);

    initEClass(wmlLuaCodeEClass, WMLLuaCode.class, "WMLLuaCode", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLLuaCode_Value(), ecorePackage.getEString(), "value", "", 0, 1, WMLLuaCode.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlMacroParameterEClass, WMLMacroParameter.class, "WMLMacroParameter", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);
    initEAttribute(getWMLMacroParameter_Value(), ecorePackage.getEString(), "value", "", 0, 1, WMLMacroParameter.class, !IS_TRANSIENT, !IS_VOLATILE, IS_CHANGEABLE, !IS_UNSETTABLE, !IS_ID, IS_UNIQUE, !IS_DERIVED, IS_ORDERED);

    initEClass(wmlValueEClass, WMLValue.class, "WMLValue", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);

    initEClass(macroTokensEClass, MacroTokens.class, "MacroTokens", !IS_ABSTRACT, !IS_INTERFACE, IS_GENERATED_INSTANCE_CLASS);

    initEClass(eSerializableEClass, Serializable.class, "ESerializable", IS_ABSTRACT, IS_INTERFACE, !IS_GENERATED_INSTANCE_CLASS);

    // Create resource
    createResource(eNS_URI);
  }

} //WmlPackageImpl
