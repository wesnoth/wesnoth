/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML;

import org.eclipse.emf.ecore.EAttribute;
import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EPackage;
import org.eclipse.emf.ecore.EReference;

/**
 * <!-- begin-user-doc -->
 * The <b>Package</b> for the model.
 * It contains accessors for the meta objects to represent
 * <ul>
 *   <li>each class,</li>
 *   <li>each feature of each class,</li>
 *   <li>each enum,</li>
 *   <li>and each data type</li>
 * </ul>
 * <!-- end-user-doc -->
 * @see org.wesnoth.wML.WMLFactory
 * @model kind="package"
 * @generated
 */
public interface WMLPackage extends EPackage
{
  /**
   * The package name.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  String eNAME = "wML";

  /**
   * The package namespace URI.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  String eNS_URI = "http://www.wesnoth.org/WML";

  /**
   * The package namespace name.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  String eNS_PREFIX = "wML";

  /**
   * The singleton instance of the package.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  WMLPackage eINSTANCE = org.wesnoth.wML.impl.WMLPackageImpl.init();

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLRootImpl <em>Root</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLRootImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLRoot()
   * @generated
   */
  int WML_ROOT = 0;

  /**
   * The feature id for the '<em><b>Tags</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT__TAGS = 0;

  /**
   * The feature id for the '<em><b>Macro Calls</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT__MACRO_CALLS = 1;

  /**
   * The number of structural features of the '<em>Root</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT_FEATURE_COUNT = 2;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLTagImpl <em>Tag</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLTagImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLTag()
   * @generated
   */
  int WML_TAG = 1;

  /**
   * The feature id for the '<em><b>Plus</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__PLUS = 0;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__NAME = 1;

  /**
   * The feature id for the '<em><b>Tags</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__TAGS = 2;

  /**
   * The feature id for the '<em><b>Keys</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__KEYS = 3;

  /**
   * The feature id for the '<em><b>Macro Calls</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__MACRO_CALLS = 4;

  /**
   * The feature id for the '<em><b>End Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__END_NAME = 5;

  /**
   * The number of structural features of the '<em>Tag</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG_FEATURE_COUNT = 6;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLKeyImpl <em>Key</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLKeyImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLKey()
   * @generated
   */
  int WML_KEY = 2;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__NAME = 0;

  /**
   * The feature id for the '<em><b>Value</b></em>' containment reference.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__VALUE = 1;

  /**
   * The feature id for the '<em><b>Extra Args</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__EXTRA_ARGS = 2;

  /**
   * The number of structural features of the '<em>Key</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY_FEATURE_COUNT = 3;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLAbstractMacroCallImpl <em>Abstract Macro Call</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLAbstractMacroCallImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLAbstractMacroCall()
   * @generated
   */
  int WML_ABSTRACT_MACRO_CALL = 3;

  /**
   * The number of structural features of the '<em>Abstract Macro Call</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ABSTRACT_MACRO_CALL_FEATURE_COUNT = 0;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLMacroIncludeImpl <em>Macro Include</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLMacroIncludeImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLMacroInclude()
   * @generated
   */
  int WML_MACRO_INCLUDE = 4;

  /**
   * The feature id for the '<em><b>Path</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_INCLUDE__PATH = WML_ABSTRACT_MACRO_CALL_FEATURE_COUNT + 0;

  /**
   * The number of structural features of the '<em>Macro Include</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_INCLUDE_FEATURE_COUNT = WML_ABSTRACT_MACRO_CALL_FEATURE_COUNT + 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLMacroCallImpl <em>Macro Call</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLMacroCallImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLMacroCall()
   * @generated
   */
  int WML_MACRO_CALL = 5;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__NAME = WML_ABSTRACT_MACRO_CALL_FEATURE_COUNT + 0;

  /**
   * The feature id for the '<em><b>Params</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__PARAMS = WML_ABSTRACT_MACRO_CALL_FEATURE_COUNT + 1;

  /**
   * The number of structural features of the '<em>Macro Call</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL_FEATURE_COUNT = WML_ABSTRACT_MACRO_CALL_FEATURE_COUNT + 2;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLAbstractKeyValueImpl <em>Abstract Key Value</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLAbstractKeyValueImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLAbstractKeyValue()
   * @generated
   */
  int WML_ABSTRACT_KEY_VALUE = 6;

  /**
   * The number of structural features of the '<em>Abstract Key Value</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ABSTRACT_KEY_VALUE_FEATURE_COUNT = 0;


  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLRoot <em>Root</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Root</em>'.
   * @see org.wesnoth.wML.WMLRoot
   * @generated
   */
  EClass getWMLRoot();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLRoot#getTags <em>Tags</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Tags</em>'.
   * @see org.wesnoth.wML.WMLRoot#getTags()
   * @see #getWMLRoot()
   * @generated
   */
  EReference getWMLRoot_Tags();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLRoot#getMacroCalls <em>Macro Calls</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macro Calls</em>'.
   * @see org.wesnoth.wML.WMLRoot#getMacroCalls()
   * @see #getWMLRoot()
   * @generated
   */
  EReference getWMLRoot_MacroCalls();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLTag <em>Tag</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Tag</em>'.
   * @see org.wesnoth.wML.WMLTag
   * @generated
   */
  EClass getWMLTag();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLTag#isPlus <em>Plus</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Plus</em>'.
   * @see org.wesnoth.wML.WMLTag#isPlus()
   * @see #getWMLTag()
   * @generated
   */
  EAttribute getWMLTag_Plus();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLTag#getName <em>Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Name</em>'.
   * @see org.wesnoth.wML.WMLTag#getName()
   * @see #getWMLTag()
   * @generated
   */
  EAttribute getWMLTag_Name();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLTag#getTags <em>Tags</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Tags</em>'.
   * @see org.wesnoth.wML.WMLTag#getTags()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_Tags();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLTag#getKeys <em>Keys</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Keys</em>'.
   * @see org.wesnoth.wML.WMLTag#getKeys()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_Keys();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLTag#getMacroCalls <em>Macro Calls</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macro Calls</em>'.
   * @see org.wesnoth.wML.WMLTag#getMacroCalls()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_MacroCalls();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLTag#getEndName <em>End Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>End Name</em>'.
   * @see org.wesnoth.wML.WMLTag#getEndName()
   * @see #getWMLTag()
   * @generated
   */
  EAttribute getWMLTag_EndName();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLKey <em>Key</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Key</em>'.
   * @see org.wesnoth.wML.WMLKey
   * @generated
   */
  EClass getWMLKey();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLKey#getName <em>Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Name</em>'.
   * @see org.wesnoth.wML.WMLKey#getName()
   * @see #getWMLKey()
   * @generated
   */
  EAttribute getWMLKey_Name();

  /**
   * Returns the meta object for the containment reference '{@link org.wesnoth.wML.WMLKey#getValue <em>Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference '<em>Value</em>'.
   * @see org.wesnoth.wML.WMLKey#getValue()
   * @see #getWMLKey()
   * @generated
   */
  EReference getWMLKey_Value();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLKey#getExtraArgs <em>Extra Args</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Extra Args</em>'.
   * @see org.wesnoth.wML.WMLKey#getExtraArgs()
   * @see #getWMLKey()
   * @generated
   */
  EReference getWMLKey_ExtraArgs();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLAbstractMacroCall <em>Abstract Macro Call</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Abstract Macro Call</em>'.
   * @see org.wesnoth.wML.WMLAbstractMacroCall
   * @generated
   */
  EClass getWMLAbstractMacroCall();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLMacroInclude <em>Macro Include</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Macro Include</em>'.
   * @see org.wesnoth.wML.WMLMacroInclude
   * @generated
   */
  EClass getWMLMacroInclude();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLMacroInclude#getPath <em>Path</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Path</em>'.
   * @see org.wesnoth.wML.WMLMacroInclude#getPath()
   * @see #getWMLMacroInclude()
   * @generated
   */
  EAttribute getWMLMacroInclude_Path();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLMacroCall <em>Macro Call</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Macro Call</em>'.
   * @see org.wesnoth.wML.WMLMacroCall
   * @generated
   */
  EClass getWMLMacroCall();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLMacroCall#getName <em>Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Name</em>'.
   * @see org.wesnoth.wML.WMLMacroCall#getName()
   * @see #getWMLMacroCall()
   * @generated
   */
  EAttribute getWMLMacroCall_Name();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLMacroCall#getParams <em>Params</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Params</em>'.
   * @see org.wesnoth.wML.WMLMacroCall#getParams()
   * @see #getWMLMacroCall()
   * @generated
   */
  EReference getWMLMacroCall_Params();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLAbstractKeyValue <em>Abstract Key Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Abstract Key Value</em>'.
   * @see org.wesnoth.wML.WMLAbstractKeyValue
   * @generated
   */
  EClass getWMLAbstractKeyValue();

  /**
   * Returns the factory that creates the instances of the model.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the factory that creates the instances of the model.
   * @generated
   */
  WMLFactory getWMLFactory();

  /**
   * <!-- begin-user-doc -->
   * Defines literals for the meta objects that represent
   * <ul>
   *   <li>each class,</li>
   *   <li>each feature of each class,</li>
   *   <li>each enum,</li>
   *   <li>and each data type</li>
   * </ul>
   * <!-- end-user-doc -->
   * @generated
   */
  interface Literals
  {
    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLRootImpl <em>Root</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLRootImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLRoot()
     * @generated
     */
    EClass WML_ROOT = eINSTANCE.getWMLRoot();

    /**
     * The meta object literal for the '<em><b>Tags</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_ROOT__TAGS = eINSTANCE.getWMLRoot_Tags();

    /**
     * The meta object literal for the '<em><b>Macro Calls</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_ROOT__MACRO_CALLS = eINSTANCE.getWMLRoot_MacroCalls();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLTagImpl <em>Tag</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLTagImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLTag()
     * @generated
     */
    EClass WML_TAG = eINSTANCE.getWMLTag();

    /**
     * The meta object literal for the '<em><b>Plus</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_TAG__PLUS = eINSTANCE.getWMLTag_Plus();

    /**
     * The meta object literal for the '<em><b>Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_TAG__NAME = eINSTANCE.getWMLTag_Name();

    /**
     * The meta object literal for the '<em><b>Tags</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__TAGS = eINSTANCE.getWMLTag_Tags();

    /**
     * The meta object literal for the '<em><b>Keys</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__KEYS = eINSTANCE.getWMLTag_Keys();

    /**
     * The meta object literal for the '<em><b>Macro Calls</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__MACRO_CALLS = eINSTANCE.getWMLTag_MacroCalls();

    /**
     * The meta object literal for the '<em><b>End Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_TAG__END_NAME = eINSTANCE.getWMLTag_EndName();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLKeyImpl <em>Key</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLKeyImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLKey()
     * @generated
     */
    EClass WML_KEY = eINSTANCE.getWMLKey();

    /**
     * The meta object literal for the '<em><b>Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_KEY__NAME = eINSTANCE.getWMLKey_Name();

    /**
     * The meta object literal for the '<em><b>Value</b></em>' containment reference feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_KEY__VALUE = eINSTANCE.getWMLKey_Value();

    /**
     * The meta object literal for the '<em><b>Extra Args</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_KEY__EXTRA_ARGS = eINSTANCE.getWMLKey_ExtraArgs();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLAbstractMacroCallImpl <em>Abstract Macro Call</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLAbstractMacroCallImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLAbstractMacroCall()
     * @generated
     */
    EClass WML_ABSTRACT_MACRO_CALL = eINSTANCE.getWMLAbstractMacroCall();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLMacroIncludeImpl <em>Macro Include</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLMacroIncludeImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLMacroInclude()
     * @generated
     */
    EClass WML_MACRO_INCLUDE = eINSTANCE.getWMLMacroInclude();

    /**
     * The meta object literal for the '<em><b>Path</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_MACRO_INCLUDE__PATH = eINSTANCE.getWMLMacroInclude_Path();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLMacroCallImpl <em>Macro Call</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLMacroCallImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLMacroCall()
     * @generated
     */
    EClass WML_MACRO_CALL = eINSTANCE.getWMLMacroCall();

    /**
     * The meta object literal for the '<em><b>Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_MACRO_CALL__NAME = eINSTANCE.getWMLMacroCall_Name();

    /**
     * The meta object literal for the '<em><b>Params</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_CALL__PARAMS = eINSTANCE.getWMLMacroCall_Params();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLAbstractKeyValueImpl <em>Abstract Key Value</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLAbstractKeyValueImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLAbstractKeyValue()
     * @generated
     */
    EClass WML_ABSTRACT_KEY_VALUE = eINSTANCE.getWMLAbstractKeyValue();

  }

} //WMLPackage
