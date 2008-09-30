#!/bin/env python
#
# Most of these tests were shamelessly taken and adopted from
# Dmitry Dvoinikov's shared lock tests. As the lock implementations
# differ, the units tests were significantly changed but logically
# are identical. As Dmitry's lock implemetation is fully recursive,
# tests which require lock recursion have been removed.
#
# Special thanks to Dmitry for having his shared lock tests available.
# This test suite made it especially easy to find the tricky corner cases
# which plagued rwlock implementation.
#

from __future__ import with_statement

import time
from rwlock import RWLock, WithWriteLock
from threading import Event, currentThread
from thread import allocate_lock as _allocate_lock



if __name__ == '__main__':
    import time
    def reader( lock, *args, **kw ):
        with lock:
            print currentThread(), "reader is reading...", str(lock)
            time.sleep( 8 )
            print "reader has completed work. Exiting..."
        
        time.sleep( 15 )
        print str(lock)
        print currentThread(), "lock released and is exiting"


    def writer( lock, *args, **kw ):
        with lock:
            print currentThread(), "writer is writer...", str(lock)
            time.sleep( 4 )

        time.sleep( 15 )
        print str(lock)
        print currentThread(), "lock released and is exiting"

##    rl = RWLock( readerBias=True, biasFactor=4 )
##    rl = RWLock( readerBias=True, biasFactor=1 )
##    rl = RWLock( readerBias=True, biasFactor=3 )
    rl = RWLock()
    wl = WithWriteLock( rl )

    import threading
    r1 = threading.Thread( name="r1", target=reader, args=(rl, 1) )
    r2 = threading.Thread( name="r2", target=reader, args=(rl, 1) )
    r3 = threading.Thread( name="r3", target=reader, args=(rl, 1) )
    r4 = threading.Thread( name="r4", target=reader, args=(rl, 1) )
    r5 = threading.Thread( name="r5", target=reader, args=(rl, 1) )
    r6 = threading.Thread( name="r6", target=reader, args=(rl, 1) )
    r7 = threading.Thread( name="r7", target=reader, args=(rl, 1) )
    r8 = threading.Thread( name="r8", target=reader, args=(rl, 1) )
    r9 = threading.Thread( name="r9", target=reader, args=(rl, 1) )
    w1 = threading.Thread( name="w1", target=writer, args=(wl, 1) )
    w2 = threading.Thread( name="w2", target=writer, args=(wl, 1) )
    w3 = threading.Thread( name="w3", target=writer, args=(wl, 1) )

    print "Starting basic reader/writer lock testing..."
    r1.start()
    r2.start()
    r3.start()
    time.sleep( 2 )
    w1.start()
    time.sleep( 2 )
    w2.start()
    r4.start()
    r5.start()
    r6.start()
    w3.start()
    time.sleep( 2 )
    r7.start()
    r8.start()
    r9.start()
    time.sleep( 15 )
    print "Main thread is waiting on lock..."
    print str(rl)
    with wl:
        print str(rl)
        print "Main thread has obtained lock and is joining on workers..."
        joinList = ( r1, r2, r3, r4, r5, r6, r7, r8, r9, w1, w2, w3 )
        for t in joinList:
            t.join()
        print "All joins completed!"

    print "Main thread has completed!"
    print "reader bias:", rl._readerBias, "factor:", rl._readerBiasFactor, "with", rl._biasedReads
    print str(rl)
    print "event count", rl.eventCount

    print "We're here, so basic testing worked! Extensively self-testing module...."
    from threading import Thread
    from time import sleep, time
    from random import random, randint
    from math import log10
    Lock = _allocate_lock


    log_lock = Lock()
    def log(s):
        log_lock.acquire()
        try:
            print s
        finally:
            log_lock.releaseWriter()

    def deadlocks(f, t):
        th = Thread(target = f)
        th.setName("Thread")
        th.setDaemon(1)
        th.start()
        th.join(t)
        return th.isAlive()

    def threads(n, *f):
        start = time()
        evt = Event()
        ths = [ Thread(target = f[i % len(f)], args = (evt, )) for i in range(n) ]
        for i, th in enumerate(ths):
            th.setDaemon(1)
            th.setName(f[i % len(f)].__name__)
            th.start()
        evt.set()
        for th in ths:
            th.join()
        return time() - start

    # simple test

    print "simple test:",

    currentThread().setName("MainThread")

    lck = RWLock()
    assert str(lck) == '<RWLock; Owner:None, Readers:[], Pending:[], Writers Pending:[]>'

    assert lck.acquireWriter()
    assert str(lck) == '<RWLock; Owner:MainThread, Readers:[], Pending:[], Writers Pending:[]>'
    lck.releaseWriter()
    assert str(lck) == '<RWLock; Owner:None, Readers:[], Pending:[], Writers Pending:[]>'

    assert lck.acquire()
    assert str(lck) == '<RWLock; Owner:None, Readers:[MainThread], Pending:[], Writers Pending:[]>'
    lck.release()
    assert str(lck) == '<RWLock; Owner:None, Readers:[], Pending:[], Writers Pending:[]>'

    try:
        lck.releaseWriter()
    except Exception, e:
        assert str(e) == 'error: release unlocked lock'
    else:
        assert False

    try:
        lck.release()
    except Exception, e:
        assert str(e) == 'error: release unlocked lock'
    else:
        assert False

    print "ok"

    # same thread shared/exclusive upgrade test

    print "same thread shared/exclusive upgrade test:",
    lck = RWLock()

    def upgrade():
        # ex -> sh <- sh <- ex
        assert str(lck) == '<RWLock; Owner:None, Readers:[], Pending:[], Writers Pending:[]>'
        assert lck.acquireWriter(), str(lck)
        assert str(lck) == '<RWLock; Owner:Thread, Readers:[], Pending:[], Writers Pending:[]>', str(lck)
        assert lck.acquire(), str(lck)
        assert str(lck) == '<RWLock; Owner:Thread, Readers:[Thread], Pending:[], Writers Pending:[]>', str(lck)
        lck.release()
        assert str(lck) == '<RWLock; Owner:Thread, Readers:[], Pending:[], Writers Pending:[]>', str(lck)
        lck.releaseWriter()
        assert str(lck) == '<RWLock; Owner:None, Readers:[], Pending:[], Writers Pending:[]>', str(lck)

        # ex -> sh <- ex <- sh
        assert str(lck) == "<RWLock; Owner:None, Readers:[], Pending:[], Writers Pending:[]>", str(lck)
        assert lck.acquireWriter(), str(lck)
        assert str(lck) == "<RWLock; Owner:Thread, Readers:[], Pending:[], Writers Pending:[]>", str(lck)
        assert lck.acquire(), str(lck)
        assert str(lck) == "<RWLock; Owner:Thread, Readers:[Thread], Pending:[], Writers Pending:[]>", str(lck)
        lck.releaseWriter()
        assert str(lck) == "<RWLock; Owner:None, Readers:[Thread], Pending:[], Writers Pending:[]>", str(lck)
        lck.release()
        assert str(lck) == "<RWLock; Owner:None, Readers:[], Pending:[], Writers Pending:[]>", str(lck)

        # sh -> ex <- ex <- sh
        assert str(lck) == "<RWLock; Owner:None, Readers:[], Pending:[], Writers Pending:[]>", str(lck)
        assert lck.acquire(), str(lck)
        assert str(lck) == "<RWLock; Owner:None, Readers:[Thread], Pending:[], Writers Pending:[]>", str(lck)
        assert lck.acquireWriter(), str(lck)
        assert str(lck) == "<RWLock; Owner:Thread, Readers:[Thread], Pending:[], Writers Pending:[]>", str(lck)
        lck.releaseWriter()
        assert str(lck) == "<RWLock; Owner:None, Readers:[Thread], Pending:[], Writers Pending:[]>", str(lck)
        lck.release()
        assert str(lck) == "<RWLock; Owner:None, Readers:[], Pending:[], Writers Pending:[]>", str(lck)

        # sh -> ex <- sh <- ex
        assert str(lck) == "<RWLock; Owner:None, Readers:[], Pending:[], Writers Pending:[]>", str(lck)
        assert lck.acquire(), str(lck)
        assert str(lck) == "<RWLock; Owner:None, Readers:[Thread], Pending:[], Writers Pending:[]>", str(lck)
        assert lck.acquireWriter(), str(lck)
        assert str(lck) == "<RWLock; Owner:Thread, Readers:[Thread], Pending:[], Writers Pending:[]>", str(lck)
        lck.release()
        assert str(lck) == "<RWLock; Owner:Thread, Readers:[], Pending:[], Writers Pending:[]>", str(lck)
        lck.releaseWriter()
        assert str(lck) == "<RWLock; Owner:None, Readers:[], Pending:[], Writers Pending:[]>", str(lck)

    assert not deadlocks(upgrade, 2.0)

    print "ok"

    # timeout test

    print "timeout test:",

    # exclusive/exclusive timeout

    lck = RWLock()
    wlck = WithWriteLock( lck )

    def f(evt):
        evt.wait()
        with wlck:
            assert str(lck) == "<RWLock; Owner:f, Readers:[], Pending:[], Writers Pending:[]>", str(lck)
            sleep(1.0)
            assert str(lck) == "<RWLock; Owner:f, Readers:[], Pending:[], Writers Pending:[g]>", str(lck)

    def g(evt):
        evt.wait()
        sleep(0.5)
        assert str(lck) == "<RWLock; Owner:f, Readers:[], Pending:[], Writers Pending:[]>", str(lck)
        assert not lck.acquireWriter(0.1)
        assert str(lck) == "<RWLock; Owner:f, Readers:[], Pending:[], Writers Pending:[]>", str(lck)
        assert lck.acquireWriter(0.5)
        assert str(lck) == "<RWLock; Owner:g, Readers:[], Pending:[], Writers Pending:[]>", str(lck)
        lck.releaseWriter()

    threads(2, f, g)
    print "ok,",

    # shared/shared no timeout

    lck = RWLock()

    def f(evt):
        evt.wait()
        with lck:
            assert str(lck) == "<RWLock; Owner:None, Readers:[f], Pending:[], Writers Pending:[]>", str(lck)
            sleep(1.0)
            assert str(lck) == "<RWLock; Owner:None, Readers:[f], Pending:[], Writers Pending:[]>", str(lck)

    def g(evt):
        evt.wait()
        sleep(0.5)
        assert str(lck) == "<RWLock; Owner:None, Readers:[f], Pending:[], Writers Pending:[]>", str(lck)
        assert lck.acquire(0.1)
        assert str(lck) == "<RWLock; Owner:None, Readers:[f,g], Pending:[], Writers Pending:[]>", str(lck)
        lck.release()

    threads(2, f, g)
    print "ok,",

    # exclusive/shared timeout

    lck = RWLock()
    wlck = WithWriteLock( lck )

    def f(evt):
        evt.wait()
        with wlck:
            assert str(lck) == "<RWLock; Owner:f, Readers:[], Pending:[], Writers Pending:[]>", str(lck)
            sleep(1.0)
            assert str(lck) == "<RWLock; Owner:f, Readers:[], Pending:[g], Writers Pending:[]>", str(lck)

    def g(evt):
        evt.wait()
        sleep(0.5)
        assert str(lck) == "<RWLock; Owner:f, Readers:[], Pending:[], Writers Pending:[]>", str(lck)
        assert not lck.acquire(0.1)
        assert str(lck) == "<RWLock; Owner:f, Readers:[], Pending:[], Writers Pending:[]>", str(lck)
        assert lck.acquire(0.5)
        assert str(lck) == "<RWLock; Owner:None, Readers:[g], Pending:[], Writers Pending:[]>", str(lck)
        lck.release()

    threads(2, f, g)
    print "ok,",

    # shared/exclusive timeout

    lck = RWLock( True )

    def f(evt):
        evt.wait()
        with lck:
            assert str(lck) == "<RWLock; Owner:None, Readers:[f], Pending:[], Writers Pending:[]>", str(lck)
            sleep(1.0)
            assert str(lck) == "<RWLock; Owner:None, Readers:[f], Pending:[], Writers Pending:[g]>", str(lck)

    def g(evt):
        evt.wait()
        sleep(0.5)
        assert str(lck) == "<RWLock; Owner:None, Readers:[f], Pending:[], Writers Pending:[]>", str(lck)
        assert not lck.acquireWriter(0.1)
        assert str(lck) == "<RWLock; Owner:None, Readers:[f], Pending:[], Writers Pending:[]>", str(lck)
        assert lck.acquireWriter(0.5)
        assert str(lck) == "<RWLock; Owner:g, Readers:[], Pending:[], Writers Pending:[]>", str(lck)
        lck.releaseWriter()

    threads(2, f, g)
    print "ok"

    # different threads shared/exclusive upgrade test

    print "different threads shared/exclusive upgrade test:",

    lck = RWLock()
    wlck = WithWriteLock( lck )

    def f(evt):
        evt.wait()

        assert str(lck) == "<RWLock; Owner:None, Readers:[], Pending:[], Writers Pending:[]>", str(lck)
        with lck:
            assert str(lck) == "<RWLock; Owner:None, Readers:[f], Pending:[], Writers Pending:[]>", str(lck)
            sleep(3.0)

    def g(evt):
        evt.wait()
        sleep(1.0)
        
        assert str(lck) == "<RWLock; Owner:None, Readers:[f], Pending:[], Writers Pending:[]>", str(lck)
        with lck:
            assert str(lck) == "<RWLock; Owner:None, Readers:[f,g], Pending:[], Writers Pending:[]>", str(lck)
            sleep(3.0)

            assert str(lck) == "<RWLock; Owner:None, Readers:[g], Pending:[], Writers Pending:[h]>", str(lck)
            with wlck:
                assert str(lck) == "<RWLock; Owner:g, Readers:[g], Pending:[], Writers Pending:[h]>", str(lck)

    def h(evt):
        evt.wait()
        sleep(2.0)
        assert str(lck) == "<RWLock; Owner:None, Readers:[f,g], Pending:[], Writers Pending:[]>", str(lck)
        with wlck:
            assert str(lck) == "<RWLock; Owner:h, Readers:[], Pending:[], Writers Pending:[]>", str(lck)
            sleep(1.0)

    threads(3, f, g, h)

    print "ok"

    # different threads exclusive/exclusive deadlock test

    print "different threads exclusive/exclusive deadlock test:",

    lck = RWLock()

    def deadlock(evt):
        lck.acquireWriter()

    assert deadlocks(lambda: threads(2, deadlock), 2.0)

    print "ok"

    # different thread shared/exclusive deadlock test

    print "different threads shared/exclusive deadlock test:",

    lck = RWLock()

    def deadlock1(evt):
        lck.acquireWriter()

    def deadlock2(evt):
        lck.acquire()

    assert deadlocks(lambda: threads(2, deadlock1, deadlock2), 2.0)

    print "ok"


    # different thread shared/shared deadlock test

    print "different threads shared/shared no deadlock test:",

    lck = RWLock( True )

    def deadlock(evt):
        lck.acquire()
            

    assert not deadlocks(lambda: threads(2, deadlock), 2.0)

    print "ok"

    # exclusive interlock + timing test

    print "exclusive interlock + serialized timing test:",

    lck = RWLock( True )
    wlck = WithWriteLock( lck )
    val = 0

    def exclusive(evt):
        evt.wait()
        global val
        for i in range(10):
            with wlck:
                assert val == 0
                val += 1
                sleep(0.05 + random() * 0.05)
                assert val == 1
                val -= 1
                sleep(0.05 + random() * 0.05)
                assert val == 0

    assert threads(4, exclusive) > 0.05 * 2 * 10 * 4

    print "ok"

    # shared non-interlock timing test

    print "shared parallel timing test:",

    lck = RWLock( True )

    def shared(evt):
        evt.wait()
        for i in range(10):
            with lck:
                sleep(0.1)

    assert threads(10, shared) < 0.1 * 10 + 4.0

    print "ok"

    # shared/exclusive test

    print "multiple exclusive/shared threads busy loops:"

    lck, shlck = RWLock(), Lock()
    wlck = WithWriteLock( lck )
    ex, sh, start, t = 0, 0, time(), 10.0
    
    def exclusive(evt):
        global ex, start, t
        evt.wait()
        i = 0
        while i % 100 != 0 or start + t > time():
            i += 1
            lck.acquireWriter()
            try:
                ex += 1

            finally:
                lck.releaseWriter()

    def shared(evt):
        global sh, start, t
        evt.wait()
        i = 0
        while i % 100 != 0 or start + t > time():
            i += 1
            lck.acquireWriter()
            try:
                with shlck:
                    sh += 1

            finally:
                lck.releaseWriter()

    # even distribution

    print "2wr/2rd:",
    ex, sh, start = 0, 0, time()
    assert 10.0 < threads(4, exclusive, exclusive, shared, shared) < 12.0
    print "%d/%d:" % (ex, sh),
    assert abs(log10(float(ex) / float(sh))) < 1.3

    print "ok"

    # exclusive starvation

    print "1wr/3rd:",
    ex, sh, start = 0, 0, time()
    assert 10.0 < threads(4, exclusive, shared, shared, shared) < 12.0
    print "%d/%d:" % (ex, sh),
    assert abs(log10(float(ex) / float(sh))) < 1.3

    print "ok"

    # shared starvation

    print "3wr/1rd:",
    ex, sh, start = 0, 0, time()
    assert 10.0 < threads(4, exclusive, exclusive, exclusive, shared) < 12.0
    print "%d/%d:" % (ex, sh),
    assert abs(log10(float(ex) / float(sh))) < 1.3

    print "ok"

    print "exhaustive timed (30-seconds) test - nonrecursive", 
    lck = RWLock()
    start, t = time(), 30.0


    def f( e ):
        global start, t
        e.wait()
        lckCnt = 0L
        lckBalance = 0L
        while start + t > time():
            # Create some locks!
            j = randint(0, 1)
            if j == 0: 
                jack = lck.acquireWriter( *(randint(0, 1) == 0 and (random()/4, ) or ()) )

            else:
                jack = lck.acquire( *(randint(0, 1) == 0 and (random()/4, ) or ()) )

            sleep( random() * 0.005 )

            # Release our lock
            if jack:
                lckBalance += 1L
                lckCnt += 1L
                if j == 0: 
                    lck.releaseWriter()
                    lckBalance -= 1L

                else:
                    lck.release()
                    lckBalance -= 1L

        assert lckBalance == 0

    f0 = lambda evt: f(evt);
    f1 = lambda evt: f(evt);
    f2 = lambda evt: f(evt);
    f3 = lambda evt: f(evt);
    f4 = lambda evt: f(evt);
    f5 = lambda evt: f(evt);
    f6 = lambda evt: f(evt);
    f7 = lambda evt: f(evt);
    f8 = lambda evt: f(evt);
    f9 = lambda evt: f(evt);

    threads( 10, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9 )
    assert str(lck) == "<RWLock; Owner:None, Readers:[], Pending:[], Writers Pending:[]>", str(lck)

    print "ok"
    print "exhaustive timed (30-seconds) test w/bias - nonrecursive", 
    lck = RWLock( True )
    start, t = time(), 30.0

    threads( 10, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9 )
    assert str(lck) == "<RWLock; Owner:None, Readers:[], Pending:[], Writers Pending:[]>", str(lck)

    print "ok"


    # heavy threading test

    # specific anti-owners scenario (users cooperate by passing the lock
    # to each other to make owner starve to death)

    print "shareds cooperate in attempt to make exclusive starve to death:",

    lck, shlck, hold = RWLock(), Lock(), 0
    evtlock, stop = Event(), Event()

    def user(evt):
        
        evt.wait()
        
        try:

            while not stop.isSet():
                with lck:
                    evtlock.set()
                    
                    with shlck:
                        global hold
                        hold += 1
                  
                    sleep(random() * 0.4)

                    waited = time()
                    while time() - waited < 3.0:
                        with shlck:
                            if hold > 1:
                                hold -= 1
                                break

                    if time() - waited >= 3.0: # but in turn they lock themselves
                        raise Exception("didn't work")

                sleep(random() * 0.1)

        except Exception, e:
            assert str(e) == "didn't work"

    def owner(evt):
        evt.wait()
        evtlock.wait()
        lck.acquireWriter()
        lck.releaseWriter()
        stop.set()

    assert not deadlocks(lambda: threads(5, owner, user, user, user, user), 10.0)

    print "ok"

    print "benchmark: writer",

    lck, ii = RWLock(), 0

    start = time()
    while time() - start < 5.0:
        for i in xrange(100):
            lck.acquireWriter()
            ii += 1
            lck.releaseWriter()

    print "%d empty lock/unlock cycles per second" % (ii / 5),

    print "ok"

    print "benchmark: writer w/wrapper",

    lck, ii = RWLock(), 0
    wlck = WithWriteLock( lck )
    
    start = time()
    while time() - start < 5.0:
        for i in xrange(100):
            with lck:
                ii += 1

    print "%d empty lock/unlock cycles per second" % (ii / 5),

    print "ok"

    print "benchmark: reader",

    lck, ii = RWLock(), 0

    start = time()
    while time() - start < 5.0:
        for i in xrange(100):
            with lck:
                ii += 1

    print "%d empty lock/unlock cycles per second" % (ii / 5),

    print "ok"

    print "benchmark: read w/bias",

    lck, ii = RWLock( True ), 0

    start = time()
    while time() - start < 5.0:
        for i in xrange(100):
            with lck:
                ii += 1

    print "%d empty lock/unlock cycles per second" % (ii / 5),

    print "ok"

    # all ok

    print "all ok"

#
