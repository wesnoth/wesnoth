from __future__ import with_statement


def callWith( *argTypes, **kw ):
    """
    Validates the call arguments match the expressed signature.
    """
    try:
        def callDecorator( f ):
            def newFunction( *args, **kw ):
                assert len(args) == len(argTypes)
                argTypeList = tuple( map( type, args ) )
                if argTypes != argTypeList:
                    raise TypeError( "Called function with '%s' but '%s' is required." % \
                                     (str(argTypeList), str(argTypes)) )

                return f( *args, **kw )
            
            newFunction.__name__ = f.__name__
            return newFunction
        
        return callDecorator

    except KeyError, key:
        raise KeyError, key + "is not a valid keyword argument"

    except TypeError, msg:
        raise TypeError, msg



def returnWith( retType, **kw ):
    """
    Validates the data return is of a specific type. If it is not,
    raise TypeError exception.
    """

    try:
        def returnDecorator( f ):
            def newFunction( *args, **kw ):
                result = f( *args, **kw )
                resultType = type( result )
                if resultType != retType:
                    raise TypeError( "Function returned '%s' but '%s' is required." % \
                                     (str(resultType), str(retType)) )
                return result

            newFunction.__name__ = f.__name__
            return newFunction

        return returnDecorator

    except KeyError, key:
        raise KeyError, key + "is not a valid keyword argument"

    except TypeError, msg:
        raise TypeError, msg



class memoized( object ):
    """
    Decorator that caches a function's return value each time it is called.
    If called later with the same arguments, the cached value is returned, and
    not re-evaluated.
    """

    def __init__( self, func, lock ):
        super( memoized, self ).__init__()
        self.__func = func
        self.__lock = lock
        self.__cache = {}
        self.__hits = 0L
        self.__misses = 0L
        self.__name__ = func.__name__
        self.__doc__ = func.__doc__
        self.__str__ = func.__str__
        self.__repr__ = func.__repr__

    def __call__( self, *args, **kw ):
        try:
            funcKey = ( args, tuple( kw.iteritems() ) )
            with self.__lock:
                result = self.__cache[ funcKey ]
                self.__hits += 1L
            return result

        except KeyError:
            funcKey = ( args, tuple( kw.iteritems() ) )
            with self.__lock:
                result = self.__cache[ funcKey ] = self.__func( *args, **kw )
                self.__misses += 1L
            return result

        except TypeError:
            # uncachable -- for instance, passing a list as an argument.
            # Better to not cache than to blow up entirely.
            with self.__lock:
                result = self.__func( *args, **kw )
                self.__misses += 1L
            return result

    def _realCall( self, *args, **kw ):
        """Bypass the caching logic directly. Statistics are unchanged."""
        with self.__lock:
            return self.__func( *args, **kw )

    def getStats( self ):
        """
        Provide cache results, as a tuple, in the form of
        (total calls, misses, hits).
        """
        with self.__lock:
            return (self.__hits + self.__misses, self.__hits, self.__misses)

    def reset( self ):
        """
        Reset the cache. Use when results of underlying function call may
        have changed.
        """
        with self.__lock:
            self.__hits = 0L
            self.__misses = 0L
            self.__cache.clear()



def withLockAndReset( funcWrite, funcCache, lock, *args, **kw ):
    def withLockAndResetCallWrapper( *args, **kw):
        with lock:
            if isinstance( funcCache, tuple ) or \
                   isinstance( funcCache, list ):
                for func in funcCache:
                    func.reset()

            else:
                funcCache.reset()

            return funcWrite( *args, **kw )

    ## Make the returned function look like the real thing
    withLockAndResetCallWrapper.__name__ = funcWrite.__name__
    withLockAndResetCallWrapper.__doc__ = funcWrite.__doc__
    withLockAndResetCallWrapper.__str__ = funcWrite.__str__
    withLockAndResetCallWrapper.__repr__ = funcWrite.__repr__
    return withLockAndResetCallWrapper


def withLock( func, lock, *args, **kw ):
    def withLockWrapper( *args, **kw ):
        with lock:
            return func( *args, **kw )
    withLockWrapper.__name__ = func.__name__
    withLockWrapper.__doc__ = func.__doc__
    withLockWrapper.__str__ = func.__str__
    withLockWrapper.__repr__ = func.__repr__
    return withLockWrapper



##class memoized( object ):
##    """
##    Decorator that caches a function's return value each time it is called.
##    If called later with the same arguments, the cached value is returned, and
##    not re-evaluated.
##    """

##    def __init__( self, func ):
##        super( memoized, self ).__init__()
##        self.__func = func
##        self.__cache = {}
##        self.__hits = 0L
##        self.__misses = 0L

##    def __repr__( self ):
##        """Return the function's docstring."""
##        return self.__func.__doc__

##    def __call__( self, *args, **kw ):
##        try:
##            funcKey = ( args, tuple( kw.iteritems() ) )
##            result = self.__cache[ funcKey ]
##            self.__hits += 1L
##            return result

##        except KeyError:
##            funcKey = ( args, tuple( kw.iteritems() ) )
##            retValue = self.__cache[ funcKey ] = self.__func( *args, **kw )
##            self.__misses += 1L
##            return retValue

##        except TypeError:
##            # uncachable -- for instance, passing a list as an argument.
##            # Better to not cache than to blow up entirely.
##            retValue = self.__func( *args, **kw )
##            self.__misses += 1L
##            return retValue

##    def _realCall( self, *args, **kw ):
##        """Bypass the caching logic directly. Statistics are unchanged."""
##        return self.__func( *args, **kw )

##    def getStats( self ):
##        """
##        Provide cache results.
##        """
##        return (self.__hits + self.__misses, self.__hits, self.__misses)

##    def reset( self ):
##        """
##        Reset the cache. Use when results of underlying function call may
##        have changed.
##        """
##        self.__cache.clear()


