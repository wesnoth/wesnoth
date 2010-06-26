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
   * The meta object id for the '{@link org.wesnoth.wML.impl.RootImpl <em>Root</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.RootImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getRoot()
   * @generated
   */
  int ROOT = 0;

  /**
   * The feature id for the '<em><b>Textdomains</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ROOT__TEXTDOMAINS = 0;

  /**
   * The feature id for the '<em><b>Preproc</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ROOT__PREPROC = 1;

  /**
   * The feature id for the '<em><b>Roots</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ROOT__ROOTS = 2;

  /**
   * The number of structural features of the '<em>Root</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ROOT_FEATURE_COUNT = 3;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.TextDomainImpl <em>Text Domain</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.TextDomainImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getTextDomain()
   * @generated
   */
  int TEXT_DOMAIN = 1;

  /**
   * The feature id for the '<em><b>Domain Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int TEXT_DOMAIN__DOMAIN_NAME = 0;

  /**
   * The number of structural features of the '<em>Text Domain</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int TEXT_DOMAIN_FEATURE_COUNT = 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.PreprocessorImpl <em>Preprocessor</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.PreprocessorImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getPreprocessor()
   * @generated
   */
  int PREPROCESSOR = 2;

  /**
   * The number of structural features of the '<em>Preprocessor</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int PREPROCESSOR_FEATURE_COUNT = 0;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.MacroImpl <em>Macro</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.MacroImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getMacro()
   * @generated
   */
  int MACRO = 3;

  /**
   * The feature id for the '<em><b>Macro Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int MACRO__MACRO_NAME = PREPROCESSOR_FEATURE_COUNT + 0;

  /**
   * The number of structural features of the '<em>Macro</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int MACRO_FEATURE_COUNT = PREPROCESSOR_FEATURE_COUNT + 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.PathIncludeImpl <em>Path Include</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.PathIncludeImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getPathInclude()
   * @generated
   */
  int PATH_INCLUDE = 4;

  /**
   * The feature id for the '<em><b>Path</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int PATH_INCLUDE__PATH = PREPROCESSOR_FEATURE_COUNT + 0;

  /**
   * The number of structural features of the '<em>Path Include</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int PATH_INCLUDE_FEATURE_COUNT = PREPROCESSOR_FEATURE_COUNT + 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.RootTypeImpl <em>Root Type</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.RootTypeImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getRootType()
   * @generated
   */
  int ROOT_TYPE = 5;

  /**
   * The feature id for the '<em><b>Start Tag</b></em>' containment reference.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ROOT_TYPE__START_TAG = 0;

  /**
   * The feature id for the '<em><b>Sub Types</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ROOT_TYPE__SUB_TYPES = 1;

  /**
   * The feature id for the '<em><b>At</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ROOT_TYPE__AT = 2;

  /**
   * The feature id for the '<em><b>Okpreproc</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ROOT_TYPE__OKPREPROC = 3;

  /**
   * The feature id for the '<em><b>End Tag</b></em>' containment reference.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ROOT_TYPE__END_TAG = 4;

  /**
   * The number of structural features of the '<em>Root Type</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ROOT_TYPE_FEATURE_COUNT = 5;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.RootTagImpl <em>Root Tag</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.RootTagImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getRootTag()
   * @generated
   */
  int ROOT_TAG = 6;

  /**
   * The feature id for the '<em><b>Tag Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ROOT_TAG__TAG_NAME = 0;

  /**
   * The number of structural features of the '<em>Root Tag</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ROOT_TAG_FEATURE_COUNT = 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.SimpleTagImpl <em>Simple Tag</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.SimpleTagImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getSimpleTag()
   * @generated
   */
  int SIMPLE_TAG = 7;

  /**
   * The feature id for the '<em><b>Tag Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int SIMPLE_TAG__TAG_NAME = ROOT_TAG__TAG_NAME;

  /**
   * The feature id for the '<em><b>End Tag</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int SIMPLE_TAG__END_TAG = ROOT_TAG_FEATURE_COUNT + 0;

  /**
   * The number of structural features of the '<em>Simple Tag</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int SIMPLE_TAG_FEATURE_COUNT = ROOT_TAG_FEATURE_COUNT + 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.AddedTagImpl <em>Added Tag</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.AddedTagImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getAddedTag()
   * @generated
   */
  int ADDED_TAG = 8;

  /**
   * The feature id for the '<em><b>Tag Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ADDED_TAG__TAG_NAME = ROOT_TAG__TAG_NAME;

  /**
   * The number of structural features of the '<em>Added Tag</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ADDED_TAG_FEATURE_COUNT = ROOT_TAG_FEATURE_COUNT + 0;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.AttributesImpl <em>Attributes</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.AttributesImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getAttributes()
   * @generated
   */
  int ATTRIBUTES = 9;

  /**
   * The feature id for the '<em><b>Attr Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ATTRIBUTES__ATTR_NAME = 0;

  /**
   * The feature id for the '<em><b>Attr Value</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ATTRIBUTES__ATTR_VALUE = 1;

  /**
   * The number of structural features of the '<em>Attributes</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ATTRIBUTES_FEATURE_COUNT = 2;


  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.Root <em>Root</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Root</em>'.
   * @see org.wesnoth.wML.Root
   * @generated
   */
  EClass getRoot();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.Root#getTextdomains <em>Textdomains</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Textdomains</em>'.
   * @see org.wesnoth.wML.Root#getTextdomains()
   * @see #getRoot()
   * @generated
   */
  EReference getRoot_Textdomains();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.Root#getPreproc <em>Preproc</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Preproc</em>'.
   * @see org.wesnoth.wML.Root#getPreproc()
   * @see #getRoot()
   * @generated
   */
  EReference getRoot_Preproc();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.Root#getRoots <em>Roots</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Roots</em>'.
   * @see org.wesnoth.wML.Root#getRoots()
   * @see #getRoot()
   * @generated
   */
  EReference getRoot_Roots();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.TextDomain <em>Text Domain</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Text Domain</em>'.
   * @see org.wesnoth.wML.TextDomain
   * @generated
   */
  EClass getTextDomain();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.TextDomain#getDomainName <em>Domain Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Domain Name</em>'.
   * @see org.wesnoth.wML.TextDomain#getDomainName()
   * @see #getTextDomain()
   * @generated
   */
  EAttribute getTextDomain_DomainName();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.Preprocessor <em>Preprocessor</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Preprocessor</em>'.
   * @see org.wesnoth.wML.Preprocessor
   * @generated
   */
  EClass getPreprocessor();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.Macro <em>Macro</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Macro</em>'.
   * @see org.wesnoth.wML.Macro
   * @generated
   */
  EClass getMacro();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.Macro#getMacroName <em>Macro Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Macro Name</em>'.
   * @see org.wesnoth.wML.Macro#getMacroName()
   * @see #getMacro()
   * @generated
   */
  EAttribute getMacro_MacroName();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.PathInclude <em>Path Include</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Path Include</em>'.
   * @see org.wesnoth.wML.PathInclude
   * @generated
   */
  EClass getPathInclude();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.PathInclude#getPath <em>Path</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Path</em>'.
   * @see org.wesnoth.wML.PathInclude#getPath()
   * @see #getPathInclude()
   * @generated
   */
  EAttribute getPathInclude_Path();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.RootType <em>Root Type</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Root Type</em>'.
   * @see org.wesnoth.wML.RootType
   * @generated
   */
  EClass getRootType();

  /**
   * Returns the meta object for the containment reference '{@link org.wesnoth.wML.RootType#getStartTag <em>Start Tag</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference '<em>Start Tag</em>'.
   * @see org.wesnoth.wML.RootType#getStartTag()
   * @see #getRootType()
   * @generated
   */
  EReference getRootType_StartTag();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.RootType#getSubTypes <em>Sub Types</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Sub Types</em>'.
   * @see org.wesnoth.wML.RootType#getSubTypes()
   * @see #getRootType()
   * @generated
   */
  EReference getRootType_SubTypes();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.RootType#getAt <em>At</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>At</em>'.
   * @see org.wesnoth.wML.RootType#getAt()
   * @see #getRootType()
   * @generated
   */
  EReference getRootType_At();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.RootType#getOkpreproc <em>Okpreproc</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Okpreproc</em>'.
   * @see org.wesnoth.wML.RootType#getOkpreproc()
   * @see #getRootType()
   * @generated
   */
  EReference getRootType_Okpreproc();

  /**
   * Returns the meta object for the containment reference '{@link org.wesnoth.wML.RootType#getEndTag <em>End Tag</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference '<em>End Tag</em>'.
   * @see org.wesnoth.wML.RootType#getEndTag()
   * @see #getRootType()
   * @generated
   */
  EReference getRootType_EndTag();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.RootTag <em>Root Tag</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Root Tag</em>'.
   * @see org.wesnoth.wML.RootTag
   * @generated
   */
  EClass getRootTag();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.RootTag#getTagName <em>Tag Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Tag Name</em>'.
   * @see org.wesnoth.wML.RootTag#getTagName()
   * @see #getRootTag()
   * @generated
   */
  EAttribute getRootTag_TagName();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.SimpleTag <em>Simple Tag</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Simple Tag</em>'.
   * @see org.wesnoth.wML.SimpleTag
   * @generated
   */
  EClass getSimpleTag();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.SimpleTag#isEndTag <em>End Tag</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>End Tag</em>'.
   * @see org.wesnoth.wML.SimpleTag#isEndTag()
   * @see #getSimpleTag()
   * @generated
   */
  EAttribute getSimpleTag_EndTag();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.AddedTag <em>Added Tag</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Added Tag</em>'.
   * @see org.wesnoth.wML.AddedTag
   * @generated
   */
  EClass getAddedTag();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.Attributes <em>Attributes</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Attributes</em>'.
   * @see org.wesnoth.wML.Attributes
   * @generated
   */
  EClass getAttributes();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.Attributes#getAttrName <em>Attr Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Attr Name</em>'.
   * @see org.wesnoth.wML.Attributes#getAttrName()
   * @see #getAttributes()
   * @generated
   */
  EAttribute getAttributes_AttrName();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.Attributes#getAttrValue <em>Attr Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Attr Value</em>'.
   * @see org.wesnoth.wML.Attributes#getAttrValue()
   * @see #getAttributes()
   * @generated
   */
  EAttribute getAttributes_AttrValue();

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
     * The meta object literal for the '{@link org.wesnoth.wML.impl.RootImpl <em>Root</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.RootImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getRoot()
     * @generated
     */
    EClass ROOT = eINSTANCE.getRoot();

    /**
     * The meta object literal for the '<em><b>Textdomains</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference ROOT__TEXTDOMAINS = eINSTANCE.getRoot_Textdomains();

    /**
     * The meta object literal for the '<em><b>Preproc</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference ROOT__PREPROC = eINSTANCE.getRoot_Preproc();

    /**
     * The meta object literal for the '<em><b>Roots</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference ROOT__ROOTS = eINSTANCE.getRoot_Roots();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.TextDomainImpl <em>Text Domain</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.TextDomainImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getTextDomain()
     * @generated
     */
    EClass TEXT_DOMAIN = eINSTANCE.getTextDomain();

    /**
     * The meta object literal for the '<em><b>Domain Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute TEXT_DOMAIN__DOMAIN_NAME = eINSTANCE.getTextDomain_DomainName();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.PreprocessorImpl <em>Preprocessor</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.PreprocessorImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getPreprocessor()
     * @generated
     */
    EClass PREPROCESSOR = eINSTANCE.getPreprocessor();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.MacroImpl <em>Macro</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.MacroImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getMacro()
     * @generated
     */
    EClass MACRO = eINSTANCE.getMacro();

    /**
     * The meta object literal for the '<em><b>Macro Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute MACRO__MACRO_NAME = eINSTANCE.getMacro_MacroName();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.PathIncludeImpl <em>Path Include</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.PathIncludeImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getPathInclude()
     * @generated
     */
    EClass PATH_INCLUDE = eINSTANCE.getPathInclude();

    /**
     * The meta object literal for the '<em><b>Path</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute PATH_INCLUDE__PATH = eINSTANCE.getPathInclude_Path();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.RootTypeImpl <em>Root Type</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.RootTypeImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getRootType()
     * @generated
     */
    EClass ROOT_TYPE = eINSTANCE.getRootType();

    /**
     * The meta object literal for the '<em><b>Start Tag</b></em>' containment reference feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference ROOT_TYPE__START_TAG = eINSTANCE.getRootType_StartTag();

    /**
     * The meta object literal for the '<em><b>Sub Types</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference ROOT_TYPE__SUB_TYPES = eINSTANCE.getRootType_SubTypes();

    /**
     * The meta object literal for the '<em><b>At</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference ROOT_TYPE__AT = eINSTANCE.getRootType_At();

    /**
     * The meta object literal for the '<em><b>Okpreproc</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference ROOT_TYPE__OKPREPROC = eINSTANCE.getRootType_Okpreproc();

    /**
     * The meta object literal for the '<em><b>End Tag</b></em>' containment reference feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference ROOT_TYPE__END_TAG = eINSTANCE.getRootType_EndTag();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.RootTagImpl <em>Root Tag</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.RootTagImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getRootTag()
     * @generated
     */
    EClass ROOT_TAG = eINSTANCE.getRootTag();

    /**
     * The meta object literal for the '<em><b>Tag Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute ROOT_TAG__TAG_NAME = eINSTANCE.getRootTag_TagName();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.SimpleTagImpl <em>Simple Tag</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.SimpleTagImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getSimpleTag()
     * @generated
     */
    EClass SIMPLE_TAG = eINSTANCE.getSimpleTag();

    /**
     * The meta object literal for the '<em><b>End Tag</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute SIMPLE_TAG__END_TAG = eINSTANCE.getSimpleTag_EndTag();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.AddedTagImpl <em>Added Tag</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.AddedTagImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getAddedTag()
     * @generated
     */
    EClass ADDED_TAG = eINSTANCE.getAddedTag();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.AttributesImpl <em>Attributes</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.AttributesImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getAttributes()
     * @generated
     */
    EClass ATTRIBUTES = eINSTANCE.getAttributes();

    /**
     * The meta object literal for the '<em><b>Attr Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute ATTRIBUTES__ATTR_NAME = eINSTANCE.getAttributes_AttrName();

    /**
     * The meta object literal for the '<em><b>Attr Value</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute ATTRIBUTES__ATTR_VALUE = eINSTANCE.getAttributes_AttrValue();

  }

} //WMLPackage
