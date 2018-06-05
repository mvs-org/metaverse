from TestCase.MVSTestCase import *
import time
import datetime


class TestSendETP(MVSTestCaseBase):
    def bt_0_send(self):

        before = time.clock()
        print "start send transaction:"

        count = 20000
        while count > 0:
            ec, message = mvs_rpc.send(Alice.Alice,Bob.password, Zac.mainaddress(), 10000, 10000, 'transaction no:'+str(count))
            if ec == 0:
                count -=1

            if count % 100 == 0:
                nowTime=datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                print "current send transaction num:%d, time:%s" %(count,nowTime)


        while True:
            Alice.mining(1)
            ec, message = mvs_rpc.get_memorypool()
            if ec == 0 and message["transcation"] == "null":
                break
        
        print "end send transaction, Elapsed time:",time.clock()-before
        
