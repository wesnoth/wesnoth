#!/usr/bin/env python
#
# A Reader/Writer lock implementation
# Copyright Greg Copeland, 2008 - 2009
# Released under GPL license for Wesnoth. See Wesnoth's
# licensing terms for this's module's license.
#
# This is a non-recursive, reader/writer (shared) lock implementation.
# Recursive use of these locks will result in deadlock. Upgrading and
# downgrading of locks (reader -> writer and writer -> reader) are possible
# for many use cases. Don't forget, if the same thread attempts to grab
# a writer lock while currently holding the same writer lock, deadlock is
# ensured.
#
# My implementation has been inspired by Dmitry Dvoinikov's <dmitry@targeted.org>
# (recusrive) shared lock implementation. Despite his implementation, my implementation
# is distinct. My implementation provides writer fifo order by default unless reader
# biases is enabled at lock creation time. My implementation attempts to honor fifo
# ordering on lock requests; thusly preventing reader/writer starvation, which is common
# to Dmitry's (and others) implementation. Likewise, my lock implementation is fully
# deterministic, with or without reader bias, while his implementation is not. My
# implementation also avoids a potential race condition, which Dmitry's potentially has
# when using timeouts.
#
# If I do say so, this implementation is pretty speedy and yet still has room for
# additional optimization.
#
from __future__ import with_statement

try:
    import thread
except ImportError:
    del _sys.modules[__name__]
    raise

from threading import Event, currentThread

__all__ = [ "RWLock", "WithWriteLock" ]
_allocate_lock = thread.allocate_lock
del thread


class RWLock( object ):
    """
    Non-recursive reader/writer (shared) lock.
    """
    def __init__( self, readerBias=False, biasFactor=2 ):
        super( RWLock, self ).__init__()
        self._readerBias = readerBias
        self._readerBiasFactor = biasFactor
        self._owner = None
        self._readers = []
        self._pendWriters = []
        self._pendReaders = []
        self._biasedReads = 0L
        self._readerBiasCount = 0L

        self.__stateLock = _allocate_lock()
        self.__eventPool = [ Event(), Event(), Event(), Event() ]
        self.eventCount = len(self.__eventPool)

    def __str__( self ):
        with self.__stateLock:
            if self._owner:
                name = self._owner.getName()

            else:
                name = 'None'

            readerNames = pendNames = writerNames = ''
            for t in self._readers:
                readerNames += t.getName() + ','
            readerNames = '[' + readerNames[:-1] + ']'
            for t, l in self._pendReaders:
                pendNames += t.getName() + ','
            pendNames = '[' + pendNames[:-1] + ']'
            for t, l in self._pendWriters:
                writerNames += t.getName() + ','
            writerNames = '[' + writerNames[:-1] + ']'
            result = "<RWLock; Owner:%s, Readers:%s, Pending:%s, Writers Pending:%s>" % \
                     (name, readerNames, pendNames, writerNames)
            return result

    def _getEventLock( self ):
        """
        Method should be called with the lock's state lock
        held.
        """
        # If we have available an event semaphore, return it; otherwise create a new one
        if self.__eventPool:
            lock = self.__eventPool.pop()

        else:
            lock = Event()

        self.eventCount += 1
        return lock

    def _returnEvent( self, lock ):
        lock.clear()
        self.__eventPool.append( lock )

    def _honorReader( self ):
        """
        Based on reader bias, determine if we will honor a writer's
        position in the lock fifo order.
        """
        retValue = False
        if self._readerBias:
            if self._readerBiasCount and \
                   self._readerBiasCount%self._readerBiasFactor == 0:
                # No reader bias - it's time for writer to advance
                self._readerBiasCount = 0L

            else:
                # Reader bias - writers lose their place this time
                self._biasedReads += 1L
                retValue = True
            self._readerBiasCount += 1

        return retValue

    def _wakeThreads( self ):
        """
        The internal state lock better be enabled before
        this method is called. Otherwise internal state
        corruption is likely.
        """
        current = currentThread()
        wakeWriter = wakeReaders = False
        if not self._owner and not self._readers:
            # No reader or writer lock in place - must advance someone
            if not self._readers and self._pendWriters and \
                   not self._readerBias:
                # No reader locks and pending writers and no reader bias
                wakeWriter = True

            elif self._pendReaders:
                # No pending writers and have pending readers
                wakeReaders = True

            elif not self._readers and self._pendWriters:
                # Now try to advance writers regardless of bias setting
                wakeWriter = True

        # Build list of locks to release
        wakeList = []
        if wakeWriter:
            self._owner, lock = self._pendWriters.pop( 0 )
            wakeList.append( lock )

        elif wakeReaders:
            for thrd, lock in self._pendReaders:
                wakeList.append( lock )
                self._readers.append( thrd )
            del self._pendReaders[:]

        if wakeList:
            for lock in wakeList:
                lock.set()

    def _waitOnLock( self, lock, timeout, readLock=True ):
        """
        Method must not be called with state lock held.
        """
        retValue = True
        if not timeout:
            lock.wait()

        else:
            lock.wait( timeout )

        current = currentThread()
        with self.__stateLock:
            retValue = lock.isSet()
            if not retValue:
                # Our lock failed
                lockTuple = (current, lock)
                if readLock and lockTuple in self._pendReaders:
                    self._pendReaders.remove( lockTuple )

                else:
                    self._pendWriters.remove( lockTuple )

                # Must always wake up threads when a failure occurs
                # as someone was likely waiting for this thread to finish.
                self._wakeThreads()

            self._returnEvent( lock )

        return retValue

    def acquire( self, timeout=None ):
        """
        Return True if lock is acquired, otherwise, return False.
        """
        lock = None
        retValue = True
        current = currentThread()
        with self.__stateLock:
            if not self._owner:
                if not self._pendWriters:
                    # No one waiting - fast path
                    self._readers.append( current )

                elif self._readers and self._honorReader():
                    # No writer lock and no one queued to get a writer lock
                    # so fast path this lock - no wait. Or, we have pending
                    # writers and fifo order will not be honored because of
                    # readerBias setting.
                    self._readers.append( current )

                else:
                    lock = self._getEventLock()
                    self._pendReaders.append( (current, lock) )

            elif current is self._owner:
                # This thread already holds a writer lock - so fast path
                # a reader lock. Writer lock is not released.
                self._readers.append( current )

            else:
                # All other cases we need to wait in line
                # Because writer lock already exists!
                lock = self._getEventLock()
                self._pendReaders.append( (current, lock) )

        if lock:
            retValue = self._waitOnLock( lock, timeout )

        return retValue

    def acquireWriter( self, timeout=None ):
        lock = None
        retValue = True
        lockTuple = None
        createLock = True
        current = currentThread()

        with self.__stateLock:
            # We will always create a lock unless no writer exists,
            # no readers, no readers pending, and no writers pending,
            # unless the only lock is a read lock granted to this thread.
            if not self._owner:
                if not self._pendReaders and \
                   not self._pendWriters:
                    # No pending readers or writers
                    if self._readers and \
                           self._readers == [current]:
                        # We have a reader lock issued but we own it - upgrade lock
                        createLock = False
                    elif not self._readers:
                        createLock = False
                        
                elif self._readers == [current]:
                    # Special lock upgrade - thread already has read lock and thread
                    # is the only reader so upgrade lock to writer.
                    createLock = False

            # For most every case, we'll have to create a lock
            # If we do, we must wait to be notified.
            if createLock:
                lock = self._getEventLock()
                lockTuple = (current, lock)
                self._pendWriters.append( lockTuple )

            else:
                # We didn't create a lock so change ownership
                self._owner = current

        if lock:
            retValue = self._waitOnLock( lock, timeout, False )

        return retValue

    def __enter__( self, timeout=None ):
        return self.acquire( timeout )

    def release( self ):
        with self.__stateLock:
            try:
                self._readers.remove( currentThread() )
                self._wakeThreads()

            except ValueError:
                raise Exception, "error: release unlocked lock"

    def releaseWriter( self ):
        with self.__stateLock:
            if currentThread() == self._owner:
                self._owner = None
                self._wakeThreads()

            else:
                raise Exception, "error: release unlocked lock"

    def __exit__( self, *args, **kw ):
        self.release()

def acquire( lock, timeout=None ):
    lock.acquire()
    return lock

def acquireWrite( lock, timeout=None ):
    lock.acquireWrite()
    return lock



class WithWriteLock( object ):
    def __init__( self, lock ):
        super( WithWriteLock, self ).__init__()
        self.__lock = lock

    def __str__( self ):
        return str( self.__lock )

    def __enter__( self, timeout=None ):
        return self.__lock.acquireWriter( timeout )

    def __exit__( self, *args, **kw ):
        return self.__lock.releaseWriter()

    def acquire( self, timeout=None ):
        return self.__lock.acquireWriter( timeout )

    def release( self ):
        return self.__lock.releaseWriter()



