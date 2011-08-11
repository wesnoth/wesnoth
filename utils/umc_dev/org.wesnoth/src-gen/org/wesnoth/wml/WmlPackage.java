/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml;

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
 * @see org.wesnoth.wml.WmlFactory
 * @model kind="package"
 * @generated
 */
public interface WmlPackage extends EPackage
{
  /**
   * The package name.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  String eNAME = "wml";

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
  String eNS_PREFIX = "wml";

  /**
   * The singleton instance of the package.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  WmlPackage eINSTANCE = org.wesnoth.wml.impl.WmlPackageImpl.init();

  /**
   * The meta object id for the '{@link java.io.Serializable <em>ESerializable</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see java.io.Serializable
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getESerializable()
   * @generated
   */
  int ESERIALIZABLE = 18;

  /**
   * The number of structural features of the '<em>ESerializable</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int ESERIALIZABLE_FEATURE_COUNT = 0;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLGrammarElementImpl <em>WML Grammar Element</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLGrammarElementImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLGrammarElement()
   * @generated
   */
  int WML_GRAMMAR_ELEMENT = 1;

  /**
   * The number of structural features of the '<em>WML Grammar Element</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_GRAMMAR_ELEMENT_FEATURE_COUNT = ESERIALIZABLE_FEATURE_COUNT + 0;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLRootImpl <em>WML Root</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLRootImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLRoot()
   * @generated
   */
  int WML_ROOT = 0;

  /**
   * The feature id for the '<em><b>Expressions</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT__EXPRESSIONS = WML_GRAMMAR_ELEMENT_FEATURE_COUNT + 0;

  /**
   * The number of structural features of the '<em>WML Root</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT_FEATURE_COUNT = WML_GRAMMAR_ELEMENT_FEATURE_COUNT + 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLValuedExpressionImpl <em>WML Valued Expression</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLValuedExpressionImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLValuedExpression()
   * @generated
   */
  int WML_VALUED_EXPRESSION = 12;

  /**
   * The number of structural features of the '<em>WML Valued Expression</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_VALUED_EXPRESSION_FEATURE_COUNT = WML_GRAMMAR_ELEMENT_FEATURE_COUNT + 0;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLExpressionImpl <em>WML Expression</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLExpressionImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLExpression()
   * @generated
   */
  int WML_EXPRESSION = 11;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_EXPRESSION__NAME = WML_VALUED_EXPRESSION_FEATURE_COUNT + 0;

  /**
   * The feature id for the '<em><b>Lua Based</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_EXPRESSION__LUA_BASED = WML_VALUED_EXPRESSION_FEATURE_COUNT + 1;

  /**
   * The feature id for the '<em><b>Definition Location</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_EXPRESSION__DEFINITION_LOCATION = WML_VALUED_EXPRESSION_FEATURE_COUNT + 2;

  /**
   * The feature id for the '<em><b>Definition Offset</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_EXPRESSION__DEFINITION_OFFSET = WML_VALUED_EXPRESSION_FEATURE_COUNT + 3;

  /**
   * The feature id for the '<em><b>Cardinality</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_EXPRESSION__CARDINALITY = WML_VALUED_EXPRESSION_FEATURE_COUNT + 4;

  /**
   * The number of structural features of the '<em>WML Expression</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_EXPRESSION_FEATURE_COUNT = WML_VALUED_EXPRESSION_FEATURE_COUNT + 5;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLRootExpressionImpl <em>WML Root Expression</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLRootExpressionImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLRootExpression()
   * @generated
   */
  int WML_ROOT_EXPRESSION = 10;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT_EXPRESSION__NAME = WML_EXPRESSION__NAME;

  /**
   * The feature id for the '<em><b>Lua Based</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT_EXPRESSION__LUA_BASED = WML_EXPRESSION__LUA_BASED;

  /**
   * The feature id for the '<em><b>Definition Location</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT_EXPRESSION__DEFINITION_LOCATION = WML_EXPRESSION__DEFINITION_LOCATION;

  /**
   * The feature id for the '<em><b>Definition Offset</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT_EXPRESSION__DEFINITION_OFFSET = WML_EXPRESSION__DEFINITION_OFFSET;

  /**
   * The feature id for the '<em><b>Cardinality</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT_EXPRESSION__CARDINALITY = WML_EXPRESSION__CARDINALITY;

  /**
   * The number of structural features of the '<em>WML Root Expression</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT_EXPRESSION_FEATURE_COUNT = WML_EXPRESSION_FEATURE_COUNT + 0;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLTagImpl <em>WML Tag</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLTagImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLTag()
   * @generated
   */
  int WML_TAG = 2;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__NAME = WML_ROOT_EXPRESSION__NAME;

  /**
   * The feature id for the '<em><b>Lua Based</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__LUA_BASED = WML_ROOT_EXPRESSION__LUA_BASED;

  /**
   * The feature id for the '<em><b>Definition Location</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__DEFINITION_LOCATION = WML_ROOT_EXPRESSION__DEFINITION_LOCATION;

  /**
   * The feature id for the '<em><b>Definition Offset</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__DEFINITION_OFFSET = WML_ROOT_EXPRESSION__DEFINITION_OFFSET;

  /**
   * The feature id for the '<em><b>Cardinality</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__CARDINALITY = WML_ROOT_EXPRESSION__CARDINALITY;

  /**
   * The feature id for the '<em><b>Plus</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__PLUS = WML_ROOT_EXPRESSION_FEATURE_COUNT + 0;

  /**
   * The feature id for the '<em><b>Expressions</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__EXPRESSIONS = WML_ROOT_EXPRESSION_FEATURE_COUNT + 1;

  /**
   * The feature id for the '<em><b>End Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__END_NAME = WML_ROOT_EXPRESSION_FEATURE_COUNT + 2;

  /**
   * The feature id for the '<em><b>Inherted Tag Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__INHERTED_TAG_NAME = WML_ROOT_EXPRESSION_FEATURE_COUNT + 3;

  /**
   * The feature id for the '<em><b>Needing Expansion</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__NEEDING_EXPANSION = WML_ROOT_EXPRESSION_FEATURE_COUNT + 4;

  /**
   * The feature id for the '<em><b>Description</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__DESCRIPTION = WML_ROOT_EXPRESSION_FEATURE_COUNT + 5;

  /**
   * The number of structural features of the '<em>WML Tag</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG_FEATURE_COUNT = WML_ROOT_EXPRESSION_FEATURE_COUNT + 6;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLKeyImpl <em>WML Key</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLKeyImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLKey()
   * @generated
   */
  int WML_KEY = 3;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__NAME = WML_EXPRESSION__NAME;

  /**
   * The feature id for the '<em><b>Lua Based</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__LUA_BASED = WML_EXPRESSION__LUA_BASED;

  /**
   * The feature id for the '<em><b>Definition Location</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__DEFINITION_LOCATION = WML_EXPRESSION__DEFINITION_LOCATION;

  /**
   * The feature id for the '<em><b>Definition Offset</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__DEFINITION_OFFSET = WML_EXPRESSION__DEFINITION_OFFSET;

  /**
   * The feature id for the '<em><b>Cardinality</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__CARDINALITY = WML_EXPRESSION__CARDINALITY;

  /**
   * The feature id for the '<em><b>Values</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__VALUES = WML_EXPRESSION_FEATURE_COUNT + 0;

  /**
   * The feature id for the '<em><b>Eol</b></em>' attribute list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__EOL = WML_EXPRESSION_FEATURE_COUNT + 1;

  /**
   * The feature id for the '<em><b>Enum</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__ENUM = WML_EXPRESSION_FEATURE_COUNT + 2;

  /**
   * The feature id for the '<em><b>Translatable</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__TRANSLATABLE = WML_EXPRESSION_FEATURE_COUNT + 3;

  /**
   * The feature id for the '<em><b>Data Type</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__DATA_TYPE = WML_EXPRESSION_FEATURE_COUNT + 4;

  /**
   * The number of structural features of the '<em>WML Key</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY_FEATURE_COUNT = WML_EXPRESSION_FEATURE_COUNT + 5;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLKeyValueImpl <em>WML Key Value</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLKeyValueImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLKeyValue()
   * @generated
   */
  int WML_KEY_VALUE = 4;

  /**
   * The number of structural features of the '<em>WML Key Value</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY_VALUE_FEATURE_COUNT = WML_GRAMMAR_ELEMENT_FEATURE_COUNT + 0;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLMacroCallImpl <em>WML Macro Call</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLMacroCallImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLMacroCall()
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
  int WML_MACRO_CALL__NAME = WML_KEY_VALUE_FEATURE_COUNT + 0;

  /**
   * The feature id for the '<em><b>Lua Based</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__LUA_BASED = WML_KEY_VALUE_FEATURE_COUNT + 1;

  /**
   * The feature id for the '<em><b>Definition Location</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__DEFINITION_LOCATION = WML_KEY_VALUE_FEATURE_COUNT + 2;

  /**
   * The feature id for the '<em><b>Definition Offset</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__DEFINITION_OFFSET = WML_KEY_VALUE_FEATURE_COUNT + 3;

  /**
   * The feature id for the '<em><b>Cardinality</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__CARDINALITY = WML_KEY_VALUE_FEATURE_COUNT + 4;

  /**
   * The feature id for the '<em><b>Point</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__POINT = WML_KEY_VALUE_FEATURE_COUNT + 5;

  /**
   * The feature id for the '<em><b>Relative</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__RELATIVE = WML_KEY_VALUE_FEATURE_COUNT + 6;

  /**
   * The feature id for the '<em><b>Parameters</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__PARAMETERS = WML_KEY_VALUE_FEATURE_COUNT + 7;

  /**
   * The number of structural features of the '<em>WML Macro Call</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL_FEATURE_COUNT = WML_KEY_VALUE_FEATURE_COUNT + 8;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLMacroCallParameterImpl <em>WML Macro Call Parameter</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLMacroCallParameterImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLMacroCallParameter()
   * @generated
   */
  int WML_MACRO_CALL_PARAMETER = 6;

  /**
   * The number of structural features of the '<em>WML Macro Call Parameter</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL_PARAMETER_FEATURE_COUNT = WML_GRAMMAR_ELEMENT_FEATURE_COUNT + 0;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLArrayCallImpl <em>WML Array Call</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLArrayCallImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLArrayCall()
   * @generated
   */
  int WML_ARRAY_CALL = 7;

  /**
   * The feature id for the '<em><b>Value</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ARRAY_CALL__VALUE = WML_KEY_VALUE_FEATURE_COUNT + 0;

  /**
   * The number of structural features of the '<em>WML Array Call</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ARRAY_CALL_FEATURE_COUNT = WML_KEY_VALUE_FEATURE_COUNT + 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLMacroDefineImpl <em>WML Macro Define</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLMacroDefineImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLMacroDefine()
   * @generated
   */
  int WML_MACRO_DEFINE = 8;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__NAME = WML_ROOT_EXPRESSION__NAME;

  /**
   * The feature id for the '<em><b>Lua Based</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__LUA_BASED = WML_ROOT_EXPRESSION__LUA_BASED;

  /**
   * The feature id for the '<em><b>Definition Location</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__DEFINITION_LOCATION = WML_ROOT_EXPRESSION__DEFINITION_LOCATION;

  /**
   * The feature id for the '<em><b>Definition Offset</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__DEFINITION_OFFSET = WML_ROOT_EXPRESSION__DEFINITION_OFFSET;

  /**
   * The feature id for the '<em><b>Cardinality</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__CARDINALITY = WML_ROOT_EXPRESSION__CARDINALITY;

  /**
   * The feature id for the '<em><b>Expressions</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__EXPRESSIONS = WML_ROOT_EXPRESSION_FEATURE_COUNT + 0;

  /**
   * The feature id for the '<em><b>End Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__END_NAME = WML_ROOT_EXPRESSION_FEATURE_COUNT + 1;

  /**
   * The number of structural features of the '<em>WML Macro Define</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE_FEATURE_COUNT = WML_ROOT_EXPRESSION_FEATURE_COUNT + 2;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLPreprocIFImpl <em>WML Preproc IF</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLPreprocIFImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLPreprocIF()
   * @generated
   */
  int WML_PREPROC_IF = 9;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__NAME = WML_ROOT_EXPRESSION__NAME;

  /**
   * The feature id for the '<em><b>Lua Based</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__LUA_BASED = WML_ROOT_EXPRESSION__LUA_BASED;

  /**
   * The feature id for the '<em><b>Definition Location</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__DEFINITION_LOCATION = WML_ROOT_EXPRESSION__DEFINITION_LOCATION;

  /**
   * The feature id for the '<em><b>Definition Offset</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__DEFINITION_OFFSET = WML_ROOT_EXPRESSION__DEFINITION_OFFSET;

  /**
   * The feature id for the '<em><b>Cardinality</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__CARDINALITY = WML_ROOT_EXPRESSION__CARDINALITY;

  /**
   * The feature id for the '<em><b>Expressions</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__EXPRESSIONS = WML_ROOT_EXPRESSION_FEATURE_COUNT + 0;

  /**
   * The feature id for the '<em><b>Elses</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__ELSES = WML_ROOT_EXPRESSION_FEATURE_COUNT + 1;

  /**
   * The feature id for the '<em><b>Else Expressions</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__ELSE_EXPRESSIONS = WML_ROOT_EXPRESSION_FEATURE_COUNT + 2;

  /**
   * The feature id for the '<em><b>End Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__END_NAME = WML_ROOT_EXPRESSION_FEATURE_COUNT + 3;

  /**
   * The number of structural features of the '<em>WML Preproc IF</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF_FEATURE_COUNT = WML_ROOT_EXPRESSION_FEATURE_COUNT + 4;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLTextdomainImpl <em>WML Textdomain</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLTextdomainImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLTextdomain()
   * @generated
   */
  int WML_TEXTDOMAIN = 13;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TEXTDOMAIN__NAME = WML_ROOT_EXPRESSION__NAME;

  /**
   * The feature id for the '<em><b>Lua Based</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TEXTDOMAIN__LUA_BASED = WML_ROOT_EXPRESSION__LUA_BASED;

  /**
   * The feature id for the '<em><b>Definition Location</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TEXTDOMAIN__DEFINITION_LOCATION = WML_ROOT_EXPRESSION__DEFINITION_LOCATION;

  /**
   * The feature id for the '<em><b>Definition Offset</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TEXTDOMAIN__DEFINITION_OFFSET = WML_ROOT_EXPRESSION__DEFINITION_OFFSET;

  /**
   * The feature id for the '<em><b>Cardinality</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TEXTDOMAIN__CARDINALITY = WML_ROOT_EXPRESSION__CARDINALITY;

  /**
   * The number of structural features of the '<em>WML Textdomain</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TEXTDOMAIN_FEATURE_COUNT = WML_ROOT_EXPRESSION_FEATURE_COUNT + 0;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLLuaCodeImpl <em>WML Lua Code</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLLuaCodeImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLLuaCode()
   * @generated
   */
  int WML_LUA_CODE = 14;

  /**
   * The feature id for the '<em><b>Value</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_LUA_CODE__VALUE = WML_KEY_VALUE_FEATURE_COUNT + 0;

  /**
   * The number of structural features of the '<em>WML Lua Code</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_LUA_CODE_FEATURE_COUNT = WML_KEY_VALUE_FEATURE_COUNT + 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLMacroParameterImpl <em>WML Macro Parameter</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLMacroParameterImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLMacroParameter()
   * @generated
   */
  int WML_MACRO_PARAMETER = 15;

  /**
   * The feature id for the '<em><b>Value</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_PARAMETER__VALUE = WML_MACRO_CALL_PARAMETER_FEATURE_COUNT + 0;

  /**
   * The number of structural features of the '<em>WML Macro Parameter</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_PARAMETER_FEATURE_COUNT = WML_MACRO_CALL_PARAMETER_FEATURE_COUNT + 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLValueImpl <em>WML Value</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLValueImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLValue()
   * @generated
   */
  int WML_VALUE = 16;

  /**
   * The feature id for the '<em><b>Value</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_VALUE__VALUE = WML_KEY_VALUE_FEATURE_COUNT + 0;

  /**
   * The number of structural features of the '<em>WML Value</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_VALUE_FEATURE_COUNT = WML_KEY_VALUE_FEATURE_COUNT + 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.MacroTokensImpl <em>Macro Tokens</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.MacroTokensImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getMacroTokens()
   * @generated
   */
  int MACRO_TOKENS = 17;

  /**
   * The feature id for the '<em><b>Value</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int MACRO_TOKENS__VALUE = WML_MACRO_PARAMETER__VALUE;

  /**
   * The number of structural features of the '<em>Macro Tokens</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int MACRO_TOKENS_FEATURE_COUNT = WML_MACRO_PARAMETER_FEATURE_COUNT + 0;


  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLRoot <em>WML Root</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Root</em>'.
   * @see org.wesnoth.wml.WMLRoot
   * @generated
   */
  EClass getWMLRoot();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLRoot#getExpressions <em>Expressions</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Expressions</em>'.
   * @see org.wesnoth.wml.WMLRoot#getExpressions()
   * @see #getWMLRoot()
   * @generated
   */
  EReference getWMLRoot_Expressions();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLGrammarElement <em>WML Grammar Element</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Grammar Element</em>'.
   * @see org.wesnoth.wml.WMLGrammarElement
   * @generated
   */
  EClass getWMLGrammarElement();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLTag <em>WML Tag</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Tag</em>'.
   * @see org.wesnoth.wml.WMLTag
   * @generated
   */
  EClass getWMLTag();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLTag#getPlus <em>Plus</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Plus</em>'.
   * @see org.wesnoth.wml.WMLTag#getPlus()
   * @see #getWMLTag()
   * @generated
   */
  EAttribute getWMLTag_Plus();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLTag#getExpressions <em>Expressions</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Expressions</em>'.
   * @see org.wesnoth.wml.WMLTag#getExpressions()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_Expressions();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLTag#getEndName <em>End Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>End Name</em>'.
   * @see org.wesnoth.wml.WMLTag#getEndName()
   * @see #getWMLTag()
   * @generated
   */
  EAttribute getWMLTag_EndName();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLTag#get_InhertedTagName <em>Inherted Tag Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Inherted Tag Name</em>'.
   * @see org.wesnoth.wml.WMLTag#get_InhertedTagName()
   * @see #getWMLTag()
   * @generated
   */
  EAttribute getWMLTag__InhertedTagName();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLTag#is_NeedingExpansion <em>Needing Expansion</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Needing Expansion</em>'.
   * @see org.wesnoth.wml.WMLTag#is_NeedingExpansion()
   * @see #getWMLTag()
   * @generated
   */
  EAttribute getWMLTag__NeedingExpansion();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLTag#get_Description <em>Description</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Description</em>'.
   * @see org.wesnoth.wml.WMLTag#get_Description()
   * @see #getWMLTag()
   * @generated
   */
  EAttribute getWMLTag__Description();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLKey <em>WML Key</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Key</em>'.
   * @see org.wesnoth.wml.WMLKey
   * @generated
   */
  EClass getWMLKey();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLKey#getValues <em>Values</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Values</em>'.
   * @see org.wesnoth.wml.WMLKey#getValues()
   * @see #getWMLKey()
   * @generated
   */
  EReference getWMLKey_Values();

  /**
   * Returns the meta object for the attribute list '{@link org.wesnoth.wml.WMLKey#getEol <em>Eol</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute list '<em>Eol</em>'.
   * @see org.wesnoth.wml.WMLKey#getEol()
   * @see #getWMLKey()
   * @generated
   */
  EAttribute getWMLKey_Eol();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLKey#is_Enum <em>Enum</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Enum</em>'.
   * @see org.wesnoth.wml.WMLKey#is_Enum()
   * @see #getWMLKey()
   * @generated
   */
  EAttribute getWMLKey__Enum();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLKey#is_Translatable <em>Translatable</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Translatable</em>'.
   * @see org.wesnoth.wml.WMLKey#is_Translatable()
   * @see #getWMLKey()
   * @generated
   */
  EAttribute getWMLKey__Translatable();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLKey#get_DataType <em>Data Type</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Data Type</em>'.
   * @see org.wesnoth.wml.WMLKey#get_DataType()
   * @see #getWMLKey()
   * @generated
   */
  EAttribute getWMLKey__DataType();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLKeyValue <em>WML Key Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Key Value</em>'.
   * @see org.wesnoth.wml.WMLKeyValue
   * @generated
   */
  EClass getWMLKeyValue();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLMacroCall <em>WML Macro Call</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Macro Call</em>'.
   * @see org.wesnoth.wml.WMLMacroCall
   * @generated
   */
  EClass getWMLMacroCall();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLMacroCall#getPoint <em>Point</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Point</em>'.
   * @see org.wesnoth.wml.WMLMacroCall#getPoint()
   * @see #getWMLMacroCall()
   * @generated
   */
  EAttribute getWMLMacroCall_Point();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLMacroCall#getRelative <em>Relative</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Relative</em>'.
   * @see org.wesnoth.wml.WMLMacroCall#getRelative()
   * @see #getWMLMacroCall()
   * @generated
   */
  EAttribute getWMLMacroCall_Relative();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLMacroCall#getParameters <em>Parameters</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Parameters</em>'.
   * @see org.wesnoth.wml.WMLMacroCall#getParameters()
   * @see #getWMLMacroCall()
   * @generated
   */
  EReference getWMLMacroCall_Parameters();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLMacroCallParameter <em>WML Macro Call Parameter</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Macro Call Parameter</em>'.
   * @see org.wesnoth.wml.WMLMacroCallParameter
   * @generated
   */
  EClass getWMLMacroCallParameter();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLArrayCall <em>WML Array Call</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Array Call</em>'.
   * @see org.wesnoth.wml.WMLArrayCall
   * @generated
   */
  EClass getWMLArrayCall();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLArrayCall#getValue <em>Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Value</em>'.
   * @see org.wesnoth.wml.WMLArrayCall#getValue()
   * @see #getWMLArrayCall()
   * @generated
   */
  EReference getWMLArrayCall_Value();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLMacroDefine <em>WML Macro Define</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Macro Define</em>'.
   * @see org.wesnoth.wml.WMLMacroDefine
   * @generated
   */
  EClass getWMLMacroDefine();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLMacroDefine#getExpressions <em>Expressions</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Expressions</em>'.
   * @see org.wesnoth.wml.WMLMacroDefine#getExpressions()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EReference getWMLMacroDefine_Expressions();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLMacroDefine#getEndName <em>End Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>End Name</em>'.
   * @see org.wesnoth.wml.WMLMacroDefine#getEndName()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EAttribute getWMLMacroDefine_EndName();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLPreprocIF <em>WML Preproc IF</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Preproc IF</em>'.
   * @see org.wesnoth.wml.WMLPreprocIF
   * @generated
   */
  EClass getWMLPreprocIF();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLPreprocIF#getExpressions <em>Expressions</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Expressions</em>'.
   * @see org.wesnoth.wml.WMLPreprocIF#getExpressions()
   * @see #getWMLPreprocIF()
   * @generated
   */
  EReference getWMLPreprocIF_Expressions();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLPreprocIF#getElses <em>Elses</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Elses</em>'.
   * @see org.wesnoth.wml.WMLPreprocIF#getElses()
   * @see #getWMLPreprocIF()
   * @generated
   */
  EAttribute getWMLPreprocIF_Elses();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLPreprocIF#getElseExpressions <em>Else Expressions</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Else Expressions</em>'.
   * @see org.wesnoth.wml.WMLPreprocIF#getElseExpressions()
   * @see #getWMLPreprocIF()
   * @generated
   */
  EReference getWMLPreprocIF_ElseExpressions();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLPreprocIF#getEndName <em>End Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>End Name</em>'.
   * @see org.wesnoth.wml.WMLPreprocIF#getEndName()
   * @see #getWMLPreprocIF()
   * @generated
   */
  EAttribute getWMLPreprocIF_EndName();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLRootExpression <em>WML Root Expression</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Root Expression</em>'.
   * @see org.wesnoth.wml.WMLRootExpression
   * @generated
   */
  EClass getWMLRootExpression();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLExpression <em>WML Expression</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Expression</em>'.
   * @see org.wesnoth.wml.WMLExpression
   * @generated
   */
  EClass getWMLExpression();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLExpression#getName <em>Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Name</em>'.
   * @see org.wesnoth.wml.WMLExpression#getName()
   * @see #getWMLExpression()
   * @generated
   */
  EAttribute getWMLExpression_Name();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLExpression#is_LuaBased <em>Lua Based</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Lua Based</em>'.
   * @see org.wesnoth.wml.WMLExpression#is_LuaBased()
   * @see #getWMLExpression()
   * @generated
   */
  EAttribute getWMLExpression__LuaBased();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLExpression#get_DefinitionLocation <em>Definition Location</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Definition Location</em>'.
   * @see org.wesnoth.wml.WMLExpression#get_DefinitionLocation()
   * @see #getWMLExpression()
   * @generated
   */
  EAttribute getWMLExpression__DefinitionLocation();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLExpression#get_DefinitionOffset <em>Definition Offset</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Definition Offset</em>'.
   * @see org.wesnoth.wml.WMLExpression#get_DefinitionOffset()
   * @see #getWMLExpression()
   * @generated
   */
  EAttribute getWMLExpression__DefinitionOffset();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLExpression#get_Cardinality <em>Cardinality</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Cardinality</em>'.
   * @see org.wesnoth.wml.WMLExpression#get_Cardinality()
   * @see #getWMLExpression()
   * @generated
   */
  EAttribute getWMLExpression__Cardinality();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLValuedExpression <em>WML Valued Expression</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Valued Expression</em>'.
   * @see org.wesnoth.wml.WMLValuedExpression
   * @generated
   */
  EClass getWMLValuedExpression();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLTextdomain <em>WML Textdomain</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Textdomain</em>'.
   * @see org.wesnoth.wml.WMLTextdomain
   * @generated
   */
  EClass getWMLTextdomain();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLLuaCode <em>WML Lua Code</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Lua Code</em>'.
   * @see org.wesnoth.wml.WMLLuaCode
   * @generated
   */
  EClass getWMLLuaCode();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLLuaCode#getValue <em>Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Value</em>'.
   * @see org.wesnoth.wml.WMLLuaCode#getValue()
   * @see #getWMLLuaCode()
   * @generated
   */
  EAttribute getWMLLuaCode_Value();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLMacroParameter <em>WML Macro Parameter</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Macro Parameter</em>'.
   * @see org.wesnoth.wml.WMLMacroParameter
   * @generated
   */
  EClass getWMLMacroParameter();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLMacroParameter#getValue <em>Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Value</em>'.
   * @see org.wesnoth.wml.WMLMacroParameter#getValue()
   * @see #getWMLMacroParameter()
   * @generated
   */
  EAttribute getWMLMacroParameter_Value();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLValue <em>WML Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Value</em>'.
   * @see org.wesnoth.wml.WMLValue
   * @generated
   */
  EClass getWMLValue();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.MacroTokens <em>Macro Tokens</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Macro Tokens</em>'.
   * @see org.wesnoth.wml.MacroTokens
   * @generated
   */
  EClass getMacroTokens();

  /**
   * Returns the meta object for class '{@link java.io.Serializable <em>ESerializable</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>ESerializable</em>'.
   * @see java.io.Serializable
   * @model instanceClass="java.io.Serializable"
   * @generated
   */
  EClass getESerializable();

  /**
   * Returns the factory that creates the instances of the model.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the factory that creates the instances of the model.
   * @generated
   */
  WmlFactory getWmlFactory();

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
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLRootImpl <em>WML Root</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLRootImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLRoot()
     * @generated
     */
    EClass WML_ROOT = eINSTANCE.getWMLRoot();

    /**
     * The meta object literal for the '<em><b>Expressions</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_ROOT__EXPRESSIONS = eINSTANCE.getWMLRoot_Expressions();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLGrammarElementImpl <em>WML Grammar Element</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLGrammarElementImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLGrammarElement()
     * @generated
     */
    EClass WML_GRAMMAR_ELEMENT = eINSTANCE.getWMLGrammarElement();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLTagImpl <em>WML Tag</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLTagImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLTag()
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
     * The meta object literal for the '<em><b>Expressions</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__EXPRESSIONS = eINSTANCE.getWMLTag_Expressions();

    /**
     * The meta object literal for the '<em><b>End Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_TAG__END_NAME = eINSTANCE.getWMLTag_EndName();

    /**
     * The meta object literal for the '<em><b>Inherted Tag Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_TAG__INHERTED_TAG_NAME = eINSTANCE.getWMLTag__InhertedTagName();

    /**
     * The meta object literal for the '<em><b>Needing Expansion</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_TAG__NEEDING_EXPANSION = eINSTANCE.getWMLTag__NeedingExpansion();

    /**
     * The meta object literal for the '<em><b>Description</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_TAG__DESCRIPTION = eINSTANCE.getWMLTag__Description();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLKeyImpl <em>WML Key</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLKeyImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLKey()
     * @generated
     */
    EClass WML_KEY = eINSTANCE.getWMLKey();

    /**
     * The meta object literal for the '<em><b>Values</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_KEY__VALUES = eINSTANCE.getWMLKey_Values();

    /**
     * The meta object literal for the '<em><b>Eol</b></em>' attribute list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_KEY__EOL = eINSTANCE.getWMLKey_Eol();

    /**
     * The meta object literal for the '<em><b>Enum</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_KEY__ENUM = eINSTANCE.getWMLKey__Enum();

    /**
     * The meta object literal for the '<em><b>Translatable</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_KEY__TRANSLATABLE = eINSTANCE.getWMLKey__Translatable();

    /**
     * The meta object literal for the '<em><b>Data Type</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_KEY__DATA_TYPE = eINSTANCE.getWMLKey__DataType();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLKeyValueImpl <em>WML Key Value</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLKeyValueImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLKeyValue()
     * @generated
     */
    EClass WML_KEY_VALUE = eINSTANCE.getWMLKeyValue();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLMacroCallImpl <em>WML Macro Call</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLMacroCallImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLMacroCall()
     * @generated
     */
    EClass WML_MACRO_CALL = eINSTANCE.getWMLMacroCall();

    /**
     * The meta object literal for the '<em><b>Point</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_MACRO_CALL__POINT = eINSTANCE.getWMLMacroCall_Point();

    /**
     * The meta object literal for the '<em><b>Relative</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_MACRO_CALL__RELATIVE = eINSTANCE.getWMLMacroCall_Relative();

    /**
     * The meta object literal for the '<em><b>Parameters</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_CALL__PARAMETERS = eINSTANCE.getWMLMacroCall_Parameters();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLMacroCallParameterImpl <em>WML Macro Call Parameter</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLMacroCallParameterImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLMacroCallParameter()
     * @generated
     */
    EClass WML_MACRO_CALL_PARAMETER = eINSTANCE.getWMLMacroCallParameter();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLArrayCallImpl <em>WML Array Call</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLArrayCallImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLArrayCall()
     * @generated
     */
    EClass WML_ARRAY_CALL = eINSTANCE.getWMLArrayCall();

    /**
     * The meta object literal for the '<em><b>Value</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_ARRAY_CALL__VALUE = eINSTANCE.getWMLArrayCall_Value();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLMacroDefineImpl <em>WML Macro Define</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLMacroDefineImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLMacroDefine()
     * @generated
     */
    EClass WML_MACRO_DEFINE = eINSTANCE.getWMLMacroDefine();

    /**
     * The meta object literal for the '<em><b>Expressions</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_DEFINE__EXPRESSIONS = eINSTANCE.getWMLMacroDefine_Expressions();

    /**
     * The meta object literal for the '<em><b>End Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_MACRO_DEFINE__END_NAME = eINSTANCE.getWMLMacroDefine_EndName();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLPreprocIFImpl <em>WML Preproc IF</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLPreprocIFImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLPreprocIF()
     * @generated
     */
    EClass WML_PREPROC_IF = eINSTANCE.getWMLPreprocIF();

    /**
     * The meta object literal for the '<em><b>Expressions</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_PREPROC_IF__EXPRESSIONS = eINSTANCE.getWMLPreprocIF_Expressions();

    /**
     * The meta object literal for the '<em><b>Elses</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_PREPROC_IF__ELSES = eINSTANCE.getWMLPreprocIF_Elses();

    /**
     * The meta object literal for the '<em><b>Else Expressions</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_PREPROC_IF__ELSE_EXPRESSIONS = eINSTANCE.getWMLPreprocIF_ElseExpressions();

    /**
     * The meta object literal for the '<em><b>End Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_PREPROC_IF__END_NAME = eINSTANCE.getWMLPreprocIF_EndName();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLRootExpressionImpl <em>WML Root Expression</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLRootExpressionImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLRootExpression()
     * @generated
     */
    EClass WML_ROOT_EXPRESSION = eINSTANCE.getWMLRootExpression();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLExpressionImpl <em>WML Expression</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLExpressionImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLExpression()
     * @generated
     */
    EClass WML_EXPRESSION = eINSTANCE.getWMLExpression();

    /**
     * The meta object literal for the '<em><b>Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_EXPRESSION__NAME = eINSTANCE.getWMLExpression_Name();

    /**
     * The meta object literal for the '<em><b>Lua Based</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_EXPRESSION__LUA_BASED = eINSTANCE.getWMLExpression__LuaBased();

    /**
     * The meta object literal for the '<em><b>Definition Location</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_EXPRESSION__DEFINITION_LOCATION = eINSTANCE.getWMLExpression__DefinitionLocation();

    /**
     * The meta object literal for the '<em><b>Definition Offset</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_EXPRESSION__DEFINITION_OFFSET = eINSTANCE.getWMLExpression__DefinitionOffset();

    /**
     * The meta object literal for the '<em><b>Cardinality</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_EXPRESSION__CARDINALITY = eINSTANCE.getWMLExpression__Cardinality();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLValuedExpressionImpl <em>WML Valued Expression</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLValuedExpressionImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLValuedExpression()
     * @generated
     */
    EClass WML_VALUED_EXPRESSION = eINSTANCE.getWMLValuedExpression();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLTextdomainImpl <em>WML Textdomain</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLTextdomainImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLTextdomain()
     * @generated
     */
    EClass WML_TEXTDOMAIN = eINSTANCE.getWMLTextdomain();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLLuaCodeImpl <em>WML Lua Code</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLLuaCodeImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLLuaCode()
     * @generated
     */
    EClass WML_LUA_CODE = eINSTANCE.getWMLLuaCode();

    /**
     * The meta object literal for the '<em><b>Value</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_LUA_CODE__VALUE = eINSTANCE.getWMLLuaCode_Value();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLMacroParameterImpl <em>WML Macro Parameter</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLMacroParameterImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLMacroParameter()
     * @generated
     */
    EClass WML_MACRO_PARAMETER = eINSTANCE.getWMLMacroParameter();

    /**
     * The meta object literal for the '<em><b>Value</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_MACRO_PARAMETER__VALUE = eINSTANCE.getWMLMacroParameter_Value();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLValueImpl <em>WML Value</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLValueImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLValue()
     * @generated
     */
    EClass WML_VALUE = eINSTANCE.getWMLValue();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.MacroTokensImpl <em>Macro Tokens</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.MacroTokensImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getMacroTokens()
     * @generated
     */
    EClass MACRO_TOKENS = eINSTANCE.getMacroTokens();

    /**
     * The meta object literal for the '{@link java.io.Serializable <em>ESerializable</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see java.io.Serializable
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getESerializable()
     * @generated
     */
    EClass ESERIALIZABLE = eINSTANCE.getESerializable();

  }

} //WmlPackage
