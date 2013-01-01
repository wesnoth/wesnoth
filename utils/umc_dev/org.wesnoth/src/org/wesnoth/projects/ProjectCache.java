/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.projects;

import java.io.EOFException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.io.StreamCorruptedException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.eclipse.core.resources.IProject;

import org.wesnoth.Logger;
import org.wesnoth.builder.DependencyListBuilder;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.preprocessor.Define;
import org.wesnoth.preprocessor.PreprocessorUtils;
import org.wesnoth.utils.ResourceUtils;
import org.wesnoth.wml.WMLConfig;
import org.wesnoth.wml.WMLTag;
import org.wesnoth.wml.WMLVariable;

/**
 * A class that stores some project specific infos
 * for current session.
 * Some of the fields of this cache can be saved to disk
 * 
 * @see ProjectCache#saveCache()
 */
public class ProjectCache implements Serializable
{
    private static final long               serialVersionUID = - 3173930983967880699L;

    private long                            definesTimestamp_;

    private transient File                  wesnothFile_;
    private transient File                  definesFile_;
    private transient IProject              project_;

    private transient Map< String, Define > defines_;
    private Map< String, String >           properties_;
    private Map< String, WMLConfig >        configFiles_;
    private DependencyListBuilder           dependTree_;
    private Map< String, WMLVariable >      variables_;

    /**
     * Creates a new {@link ProjectCache}
     * 
     * @param project
     *        The project the cache is created for
     */
    public ProjectCache( IProject project )
    {
        project_ = project;

        configFiles_ = new HashMap< String, WMLConfig >( );
        defines_ = new HashMap< String, Define >( );
        variables_ = new HashMap< String, WMLVariable >( );
        properties_ = new HashMap< String, String >( );

        dependTree_ = new DependencyListBuilder( project_ );

        definesTimestamp_ = - 1;

        wesnothFile_ = new File( project.getLocation( ).toOSString( )
            + "/.wesnoth" ); //$NON-NLS-1$
        definesFile_ = new File( PreprocessorUtils.getInstance( )
            .getMacrosLocation( project ) );
    }

    /**
     * Returns the associated project for this cache
     * 
     * @return An IProject instance
     */
    public IProject getProject( )
    {
        return project_;
    }

    /**
     * Gets the properties map for this project.
     * 
     * @return A map with properties of the project
     */
    public Map< String, String > getProperties( )
    {
        return properties_;
    }

    /**
     * Gets the map with the WMLConfigs
     * The key represent the filenames of the files
     * and the value the scenarioId from that file
     * 
     * @return A map with key the file path and value the WMLConfig
     */
    public Map< String, WMLConfig > getWMLConfigs( )
    {
        return configFiles_;
    }

    /**
     * Gets the WMLConfig by the specified file project-relative path.
     * If the WMLConfig doesn't exist it will be created.
     * 
     * @param path
     *        The project-relative path for the file.
     * @return A {@link WMLConfig} instance. It will never be null.
     */
    public WMLConfig getWMLConfig( String path )
    {
        WMLConfig config = configFiles_.get( path );
        if( config == null ) {
            config = new WMLConfig( path );
            configFiles_.put( path, config );
        }

        return config;
    }

    /**
     * Returns the variables found in this project
     * 
     * @return A multimap containing all the variables
     */
    public Map< String, WMLVariable > getVariables( )
    {
        return variables_;
    }

    /**
     * Loads this class from the {@code .wesnoth} file and
     * loads the other cached files
     */
    public void loadCache( )
    {
        ResourceUtils
            .createWesnothFile( wesnothFile_.getAbsolutePath( ), false );

        FileInputStream inputStream = null;
        ObjectInputStream deserializer = null;
        try {
            inputStream = new FileInputStream( wesnothFile_ );
            deserializer = new ObjectInputStream(
                inputStream );
            ProjectCache cache = ( ProjectCache ) deserializer.readObject( );

            properties_ = cache.properties_;
            configFiles_ = cache.configFiles_;
            dependTree_ = cache.dependTree_;
            variables_ = cache.variables_;

            dependTree_.deserialize( project_ );

            deserializer.close( );
            inputStream.close( );
        }
        // invalid file contents. just save this instance to it.
        catch( EOFException e ) {
            saveCache( );
        } catch( StreamCorruptedException e ) {
            saveCache( );
        } catch( ClassCastException ex ) {
            saveCache( );
        } catch( ClassNotFoundException e ) {
            saveCache( );
        } catch( IOException e ) {
            Logger.getInstance( ).logException( e );
        } finally {
            try {
                if( deserializer != null ) {
                    deserializer.close( );
                }

                if( inputStream != null ) {
                    inputStream.close( );
                }
            } catch( IOException e ) {
                Logger.getInstance( ).logException( e );
            }
        }

        readDefines( true );
    }

    /**
     * Saves the cache to disk.
     * Saves:
     * - properties
     * - existing scenarios
     * 
     * @return True if the cache was sucessfully saved, false otherwise.
     */
    public boolean saveCache( )
    {
        ResourceUtils
            .createWesnothFile( wesnothFile_.getAbsolutePath( ), false );

        FileOutputStream outputStream = null;
        ObjectOutputStream serializer = null;
        try {
            outputStream = new FileOutputStream( wesnothFile_ );
            serializer = new ObjectOutputStream( outputStream );
            serializer.writeObject( this );

            serializer.close( );

            outputStream.close( );
            return true;
        } catch( Exception e ) {
            Logger.getInstance( ).logException( e );
            return false;
        } finally {
            try {
                if( serializer != null ) {
                    serializer.close( );
                }

                if( outputStream != null ) {
                    outputStream.close( );
                }
            } catch( IOException e ) {
                Logger.getInstance( ).logException( e );
            }
        }
    }

    /**
     * Reads the defines files for this project
     * 
     * @param force
     *        Read the defines even if the defines file's contents
     *        haven't changed since last time read.
     */
    public void readDefines( boolean force )
    {
        if( force == false && definesFile_.lastModified( ) <= definesTimestamp_ ) {
            return;
        }

        if( definesFile_.exists( ) == false ) {
            return;
        }

        defines_ = Define.readDefines( definesFile_.getAbsolutePath( ) );
        definesTimestamp_ = definesFile_.lastModified( );
    }

    /**
     * Returns the defines associated with this project
     * 
     * @return A {@link Map} with the defines
     */
    public Map< String, Define > getDefines( )
    {
        return defines_;
    }

    /**
     * @return The name of the install used in the project
     */
    public String getInstallName( )
    {
        return Preferences.getString( Preferences.INST_NAME_PREFIX
            + project_.getName( ) );
    }

    /**
     * Sets the new install used in the project
     * 
     * @param newInstallName
     *        The new install name
     */
    public void setInstallName( String newInstallName )
    {
        Preferences.getPreferences( ).setValue(
            Preferences.INST_NAME_PREFIX + project_.getName( ),
            newInstallName );
    }

    /**
     * Returns the current dependency tree builder for this project
     * 
     * @return A dependency tree
     */
    public DependencyListBuilder getDependencyList( )
    {
        return dependTree_;
    }

    /**
     * Clears all the project cache
     */
    public void clear( )
    {
        properties_.clear( );
        configFiles_.clear( );
        defines_.clear( );
        variables_.clear( );
        dependTree_ = new DependencyListBuilder( project_ );

        definesTimestamp_ = - 1;

        saveCache( );
    }

    /**
     * Returns the parsed Event names from the config files
     * 
     * @return A set with event names
     */
    public Set< String > getEvents( )
    {
        Set< String > result = new HashSet< String >( );

        for( WMLConfig config: configFiles_.values( ) ) {
            result.addAll( config.getEvents( ) );
        }

        return result;
    }

    /**
     * Returns the parsed WML Tags from all configs of this project
     * 
     * @return A list of Tags
     */
    public Map< String, WMLTag > getWMLTags( )
    {
        Map< String, WMLTag > res = new HashMap< String, WMLTag >( );
        for( WMLConfig config: configFiles_.values( ) ) {
            res.putAll( config.getWMLTags( ) );
        }

        return res;
    }
}
