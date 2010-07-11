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
   * The feature id for the '<em><b>Rtags</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT__RTAGS = 0;

  /**
   * The feature id for the '<em><b>Rmacros</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT__RMACROS = 1;

  /**
   * The number of structural features of the '<em>Root</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT_FEATURE_COUNT = 2;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLMacroImpl <em>Macro</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLMacroImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLMacro()
   * @generated
   */
  int WML_MACRO = 1;

  /**
   * The feature id for the '<em><b>Tagcontent</b></em>' attribute list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO__TAGCONTENT = 0;

  /**
   * The number of structural features of the '<em>Macro</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_FEATURE_COUNT = 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLTagImpl <em>Tag</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLTagImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLTag()
   * @generated
   */
  int WML_TAG = 2;

  /**
   * The feature id for the '<em><b>Start</b></em>' containment reference.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__START = 0;

  /**
   * The feature id for the '<em><b>Ttags</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__TTAGS = 1;

  /**
   * The feature id for the '<em><b>Tkeys</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__TKEYS = 2;

  /**
   * The feature id for the '<em><b>Tmacros</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__TMACROS = 3;

  /**
   * The feature id for the '<em><b>End</b></em>' containment reference.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__END = 4;

  /**
   * The number of structural features of the '<em>Tag</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG_FEATURE_COUNT = 5;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLStartTagImpl <em>Start Tag</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLStartTagImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLStartTag()
   * @generated
   */
  int WML_START_TAG = 3;

  /**
   * The feature id for the '<em><b>Tagname</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_START_TAG__TAGNAME = 0;

  /**
   * The number of structural features of the '<em>Start Tag</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_START_TAG_FEATURE_COUNT = 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLEndTagImpl <em>End Tag</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLEndTagImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLEndTag()
   * @generated
   */
  int WML_END_TAG = 4;

  /**
   * The feature id for the '<em><b>Tagname</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_END_TAG__TAGNAME = 0;

  /**
   * The number of structural features of the '<em>End Tag</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_END_TAG_FEATURE_COUNT = 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLKeyImpl <em>Key</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLKeyImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLKey()
   * @generated
   */
  int WML_KEY = 5;

  /**
   * The feature id for the '<em><b>Key Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__KEY_NAME = 0;

  /**
   * The feature id for the '<em><b>Key Value</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__KEY_VALUE = 1;

  /**
   * The number of structural features of the '<em>Key</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY_FEATURE_COUNT = 2;


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
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLRoot#getRtags <em>Rtags</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Rtags</em>'.
   * @see org.wesnoth.wML.WMLRoot#getRtags()
   * @see #getWMLRoot()
   * @generated
   */
  EReference getWMLRoot_Rtags();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLRoot#getRmacros <em>Rmacros</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Rmacros</em>'.
   * @see org.wesnoth.wML.WMLRoot#getRmacros()
   * @see #getWMLRoot()
   * @generated
   */
  EReference getWMLRoot_Rmacros();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLMacro <em>Macro</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Macro</em>'.
   * @see org.wesnoth.wML.WMLMacro
   * @generated
   */
  EClass getWMLMacro();

  /**
   * Returns the meta object for the attribute list '{@link org.wesnoth.wML.WMLMacro#getTagcontent <em>Tagcontent</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute list '<em>Tagcontent</em>'.
   * @see org.wesnoth.wML.WMLMacro#getTagcontent()
   * @see #getWMLMacro()
   * @generated
   */
  EAttribute getWMLMacro_Tagcontent();

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
   * Returns the meta object for the containment reference '{@link org.wesnoth.wML.WMLTag#getStart <em>Start</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference '<em>Start</em>'.
   * @see org.wesnoth.wML.WMLTag#getStart()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_Start();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLTag#getTtags <em>Ttags</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Ttags</em>'.
   * @see org.wesnoth.wML.WMLTag#getTtags()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_Ttags();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLTag#getTkeys <em>Tkeys</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Tkeys</em>'.
   * @see org.wesnoth.wML.WMLTag#getTkeys()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_Tkeys();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLTag#getTmacros <em>Tmacros</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Tmacros</em>'.
   * @see org.wesnoth.wML.WMLTag#getTmacros()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_Tmacros();

  /**
   * Returns the meta object for the containment reference '{@link org.wesnoth.wML.WMLTag#getEnd <em>End</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference '<em>End</em>'.
   * @see org.wesnoth.wML.WMLTag#getEnd()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_End();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLStartTag <em>Start Tag</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Start Tag</em>'.
   * @see org.wesnoth.wML.WMLStartTag
   * @generated
   */
  EClass getWMLStartTag();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLStartTag#getTagname <em>Tagname</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Tagname</em>'.
   * @see org.wesnoth.wML.WMLStartTag#getTagname()
   * @see #getWMLStartTag()
   * @generated
   */
  EAttribute getWMLStartTag_Tagname();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLEndTag <em>End Tag</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>End Tag</em>'.
   * @see org.wesnoth.wML.WMLEndTag
   * @generated
   */
  EClass getWMLEndTag();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLEndTag#getTagname <em>Tagname</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Tagname</em>'.
   * @see org.wesnoth.wML.WMLEndTag#getTagname()
   * @see #getWMLEndTag()
   * @generated
   */
  EAttribute getWMLEndTag_Tagname();

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
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLKey#getKeyName <em>Key Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Key Name</em>'.
   * @see org.wesnoth.wML.WMLKey#getKeyName()
   * @see #getWMLKey()
   * @generated
   */
  EAttribute getWMLKey_KeyName();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLKey#getKeyValue <em>Key Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Key Value</em>'.
   * @see org.wesnoth.wML.WMLKey#getKeyValue()
   * @see #getWMLKey()
   * @generated
   */
  EAttribute getWMLKey_KeyValue();

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
     * The meta object literal for the '<em><b>Rtags</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_ROOT__RTAGS = eINSTANCE.getWMLRoot_Rtags();

    /**
     * The meta object literal for the '<em><b>Rmacros</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_ROOT__RMACROS = eINSTANCE.getWMLRoot_Rmacros();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLMacroImpl <em>Macro</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLMacroImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLMacro()
     * @generated
     */
    EClass WML_MACRO = eINSTANCE.getWMLMacro();

    /**
     * The meta object literal for the '<em><b>Tagcontent</b></em>' attribute list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_MACRO__TAGCONTENT = eINSTANCE.getWMLMacro_Tagcontent();

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
     * The meta object literal for the '<em><b>Start</b></em>' containment reference feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__START = eINSTANCE.getWMLTag_Start();

    /**
     * The meta object literal for the '<em><b>Ttags</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__TTAGS = eINSTANCE.getWMLTag_Ttags();

    /**
     * The meta object literal for the '<em><b>Tkeys</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__TKEYS = eINSTANCE.getWMLTag_Tkeys();

    /**
     * The meta object literal for the '<em><b>Tmacros</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__TMACROS = eINSTANCE.getWMLTag_Tmacros();

    /**
     * The meta object literal for the '<em><b>End</b></em>' containment reference feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__END = eINSTANCE.getWMLTag_End();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLStartTagImpl <em>Start Tag</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLStartTagImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLStartTag()
     * @generated
     */
    EClass WML_START_TAG = eINSTANCE.getWMLStartTag();

    /**
     * The meta object literal for the '<em><b>Tagname</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_START_TAG__TAGNAME = eINSTANCE.getWMLStartTag_Tagname();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLEndTagImpl <em>End Tag</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLEndTagImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLEndTag()
     * @generated
     */
    EClass WML_END_TAG = eINSTANCE.getWMLEndTag();

    /**
     * The meta object literal for the '<em><b>Tagname</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_END_TAG__TAGNAME = eINSTANCE.getWMLEndTag_Tagname();

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
     * The meta object literal for the '<em><b>Key Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_KEY__KEY_NAME = eINSTANCE.getWMLKey_KeyName();

    /**
     * The meta object literal for the '<em><b>Key Value</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_KEY__KEY_VALUE = eINSTANCE.getWMLKey_KeyValue();

  }

} //WMLPackage
