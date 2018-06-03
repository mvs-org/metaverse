#!/usr/bin/python
# -*- coding: utf-8 -*-
import os, time
from utils import mvs_rpc, common
import MOCs

class Role:
    def __init__(self, name, mnemonic, addresslist, keystore_file):
        self.name = name
        self.password = name[0] + "123456"
        self.mnemonic = mnemonic.split()
        self.addresslist = addresslist
        self.keystore_file = keystore_file

        self.addresslist.reverse()
        self.did_symbol = (name+".DIID").upper()
        self.did_address = None
        self.asset_symbol = ""
        self.domain_symbol = ""
        self.multisig_addresses = {} # desc : multisig-addr

    def ensure_balance(self, mount=500):
        try:
            while self.get_balance() < mount * (10**8):
                self.mining(50)
        finally:
            print("ensure balance of {}".format(self.name))

    def lastword(self):
        return self.mnemonic[-1]

    def mainaddress(self):
        return self.addresslist[0]

    def didaddress(self):
        if None == self.did_address:
            ec, message = mvs_rpc.list_dids(self.name, self.password)
            assert (ec == 0)
            if message['dids']:
                dids = [MOCs.Did.init(i) for i in message["dids"] if i]
                found_dids = filter(lambda a: a.symbol == self.did_symbol, dids)
                assert(len(found_dids) == 1)
                self.did_address = found_dids[0].address

        return self.did_address

    def create(self):
        '''
        create account by importkeyfile
        '''
        #auto create a new asset name for each time create_asset is called
        self.domain_symbol = (self.name + common.get_random_str()).upper()
        self.asset_symbol = (self.domain_symbol + ".AST." + common.get_random_str()).upper()
        return mvs_rpc.import_keyfile(self.name, self.password, self.keystore_file)

    def dump_keyfile(self, path):
        return mvs_rpc.dump_keyfile(self.name, self.password, self.lastword(), path)

    def delete(self):
        '''
        delete account by deleteaccount
        '''
        return mvs_rpc.delete_account(self.name, self.password, self.lastword())

    def get_balance(self):
        ec, message = mvs_rpc.get_balance(self.name, self.password)
        assert (ec == 0)
        return message['total-available'] # spendable

    def register_did(self,address=None,symbol=None):
        '''
        issue did to the main address.
        '''
        if address == None:
            address = self.mainaddress()
        if symbol == None:
            symbol = self.did_symbol

        return mvs_rpc.register_did(self.name, self.password, address, symbol)

    def create_random_asset(self, domain_symbol=None, did_symbol=None, is_issue=True, secondary=0):
        if None == domain_symbol:
            domain_symbol = (self.name + common.get_random_str()).upper()
        asset_symbol = domain_symbol + ".AST"
        self.create_asset_with_symbol(asset_symbol, is_issue, secondary, did_symbol)
        return domain_symbol, asset_symbol

    def create_asset_with_symbol(self, symbol, is_issue=True, secondary=0, did_symbol=None):
        if did_symbol == None:
            did_symbol = self.did_symbol
        result, message = mvs_rpc.create_asset(self.name, self.password, symbol, 300000, did_symbol, description="%s's Asset" % self.name, rate=secondary)
        if (result):
            print("#ERROR#: failed to create asset: {}".format(message))
        assert (result == 0)
        if is_issue:
            result, message = mvs_rpc.issue_asset(self.name, self.password, symbol)
            if (result):
                print("#ERROR#: failed to issue asset: {}".format(message))
            assert (result == 0)
        return result, message

    def issue_asset_with_symbol(self, symbol, attenuation_model=None):
        return mvs_rpc.issue_asset(self.name, self.password, symbol, model=attenuation_model)

    def secondary_issue_asset_with_symbol(self, symbol, attenuation_model=None, volume=None):
        if volume == None:
            volume = 300000
        return mvs_rpc.secondary_issue(self.name, self.password, self.did_symbol, symbol, volume=volume, model=attenuation_model, fee=None)

    def create_asset(self, is_issue=True, secondary=0):
        '''
        issue asset to the main address.
        secondary:   The rate of secondaryissue. Default to 0, means the
                     asset is not allowed to secondary issue forever;
                     otherwise, -1 means the asset can be secondary issue
                     freely; otherwise, the valid rate is in range of 1
                     to 100, means the asset can be secondary issue when
                     own percentage greater than or equal to the rate
                     value.
        '''
        self.domain_symbol = (self.name + common.get_random_str()).upper();
        self.asset_symbol = (self.domain_symbol + ".AST." + common.get_random_str()).upper()
        result, message = mvs_rpc.create_asset(self.name, self.password, self.asset_symbol, 300000, self.did_symbol, description="%s's Asset" % self.name, rate=secondary)
        assert (result == 0)
        if is_issue:
            result, message = mvs_rpc.issue_asset(self.name, self.password, self.asset_symbol)
            assert (result == 0)

    def issue_cert(self, to_):
        cert_symbol = (self.name + ".2%s." % to_.name + common.get_random_str()).upper()
        result, message = mvs_rpc.issue_cert(self.name, self.password, to_.did_symbol, cert_symbol, "NAMING")
        if result != 0:
            print("failed to issue_cert: {}".format(message))
        assert (result == 0)
        return cert_symbol

    def issue_naming_cert(self, to_, domain_symbol):
        cert_symbol = (domain_symbol + ".2%s." % to_.name + common.get_random_str()).upper()
        result, message = mvs_rpc.issue_cert(self.name, self.password, to_.did_symbol, cert_symbol, "NAMING")
        if result != 0:
            print("failed to issue_cert: {}".format(message))
        assert (result == 0)
        return cert_symbol

    def delete_localasset(self, asset_symbol=None):
        if None == asset_symbol:
            asset_symbol = self.asset_symbol
        result, message = mvs_rpc.delete_localasset(self.name, self.password, asset_symbol)
        assert (result == 0)

    @classmethod
    def get_asset(cls, asset_symbol=None, cert=False):
        result, message = mvs_rpc.get_asset(asset_symbol)
        assert (result == 0)
        if cert:
            if message["assetcerts"]:
                return [MOCs.Cert.init(i) for i in message["assetcerts"] if i]
        else:
            if message["assets"]:
                return [MOCs.Asset.init(i) for i in message["assets"] if i]
        return []

    def get_accountasset(self, asset_symbol=None, cert=False):
        if None == asset_symbol:
            asset_symbol = self.asset_symbol
        result, message = mvs_rpc.get_accountasset(self.name, self.password, asset_symbol)
        assert (result == 0)
        if cert:
            if message["assetcerts"]:
                return [MOCs.Cert.init(i) for i in message["assetcerts"] if i]
        else:
            if message["assets"]:
                return [MOCs.Asset.init(i) for i in message["assets"] if i]
        return []

    @classmethod
    def get_addressasset(cls, address, cert=False):
        result, message = mvs_rpc.get_addressasset(address, cert)
        assert (result == 0)
        if cert:
            if message["assetcerts"]:
                return [MOCs.Cert.init(i) for i in message["assetcerts"] if i]
        else:
            if message["assets"]:
                return [MOCs.Asset.init(i) for i in message["assets"] if i]
        return []

    def send_asset(self, to_, amount, asset_symbol=None):
        if not asset_symbol:
            asset_symbol = self.asset_symbol
        result, message = mvs_rpc.send_asset(self.name, self.password, to_, asset_symbol, amount)
        if (result != 0):
            print("failed to send_asset: {}, {}".format(result, message))
        assert (result == 0)

    def send_asset_from(self, from_, to_, amount, asset_symbol=None):
        if not asset_symbol:
            asset_symbol = self.asset_symbol
        result, message = mvs_rpc.send_asset_from(self.name, self.password, from_, to_, asset_symbol, amount)
        assert (result == 0)

    def burn_asset(self, amount, asset_symbol=None):
        if not asset_symbol:
            asset_symbol = self.asset_symbol
        result, message = mvs_rpc.burn(self.name, self.password, asset_symbol, amount)
        assert (result == 0)

    def send_etp(self, to_, amount):
        result, message = mvs_rpc.send(self.name, self.password, to_, amount)
        assert (result == 0)
        return message["transaction"]["hash"]

    def sendmore_etp(self, receivers):
        result, message = mvs_rpc.sendmore(self.name, self.password, receivers)
        assert (result == 0)

    def didsend_etp(self, to_, amount):
        '''
        :param to_: to did/address
        '''
        result, message = mvs_rpc.didsend(self.name, self.password, to_, amount)
        assert (result == 0)
        return message["transaction"]["hash"]

    def didsend_etp_from(self, from_, to_, amount):
        '''
        :param from_: did/address
        :param to_: did/address
        '''
        result, message = mvs_rpc.didsend_from(self.name, self.password, from_, to_, amount)
        assert (result == 0)
        return message["transaction"]["hash"]

    def didsend_asset(self, to_, amount, symbol):
        result, message = mvs_rpc.didsend_asset(self.name, self.password, to_, symbol, amount)
        assert (result == 0)
        return message["transaction"]["hash"]

    def didsend_asset_from(self, from_, to_, amount, symbol):
        result, message = mvs_rpc.didsend_asset_from(self.name, self.password, from_, to_, symbol, amount)
        assert (result == 0)
        return message["transaction"]["hash"]

    def mining(self, times=1):
        '''
        use the mainaddress to mining x times.
        do mining to get the main address rich.

        result, (height_origin, _) = mvs_rpc.get_info()
        assert (result == 0)
        mvs_rpc.start_mining(self.name, self.password, self.mainaddress(), times)
        for i in range(10):
            time.sleep(0.1)
            result, (height_new, _) = mvs_rpc.get_info()
            assert (result == 0)
            if height_new == (height_origin + times):
                break

        return
        '''
        from ethereum.pow.ethpow import mine
        result, message = mvs_rpc.set_miningaccount(
            self.name,
            self.password,
            self.mainaddress()
        )
        assert(result == 0)

        def __mine__():
            result, (header_hash, seed_hash, boundary) = mvs_rpc.eth_get_work()
            assert (result == 0)
            result, (height, difficulty) = mvs_rpc.get_info()
            assert (result == 0)

            rounds = 100
            nonce = 0
            while True:
                bin_nonce, mixhash = mine(block_number=height+1, difficulty=difficulty, mining_hash=header_hash,
                                          rounds=rounds, start_nonce=nonce)
                if bin_nonce:
                    break
                nonce += rounds
            return bin_nonce, '0x' + common.toString(header_hash), '0x' + common.toString(mixhash)

        for i in xrange(times):
            bin_nonce, header_hash, mix_hash = __mine__()
            result, message = mvs_rpc.eth_submit_work('0x' + common.toString(bin_nonce), header_hash, mix_hash)
            assert (result == 0)

    def new_multisigaddress(self, description, others, required_key_num):
        '''
        don't use the same description for different multisig_addresses.
        '''
        result, message = mvs_rpc.getnew_multisig(
            self.name,
            self.password,
            description,
            self.get_publickey( self.mainaddress() ),
            [i.get_publickey( i.mainaddress() ) for i in others],
            required_key_num)
        assert (result == 0)
        assert(message["description"] == description)
        self.multisig_addresses[description] = message["address"]
        return message["address"]

    def get_publickey(self, address):
        result, publickey = mvs_rpc.get_publickey(self.name, self.password, address)
        assert (result == 0)
        return publickey

    def get_multisigaddress(self, description):
        return self.multisig_addresses[description]

    def get_didaddress(self, symbol):
        ec, message = mvs_rpc.list_didaddresses(symbol)
        assert(ec == 0)
        return message['addresses'][0]['address']

    def register_mit(self, to_did, symbol=None, content=None, fee=None):
        if None == symbol:
            symbol = common.get_random_str()
        if None == to_did:
            to_did = self.did_symbol
        ec, message = mvs_rpc.register_mit(self.name, self.password, to_did, symbol, content, fee)
        assert(ec == 0)
        return symbol

    def transfer_mit(self, to_did, symbol, fee=None):
        return mvs_rpc.transfer_mit(self.name, self.password, to_did, symbol, fee)

    def list_mits(self, name=None, password=None):
        return mvs_rpc.list_mits(name, password)

    def get_mit(self, symbol=None, trace=False):
        return mvs_rpc.get_mit(symbol, trace)

class NewGuy(Role):
    '''
    New guy means:
         1. account is newly created by getnewaccount each time create() is called;
         2. this new guy always have no money after created.
         3. the most effient way to get money is some other(Alice/Bob/...) send etp to him
    '''

    def __init__(self, name):
        Role.__init__(self, name, "", [], "")


    def create(self):
        '''
        create account by getnewaccount
        '''
        #auto create a new asset name for each time create_asset is called
        self.asset_symbol = (self.name + ".AST." + common.get_random_str()).upper()

        result, self.mnemonic = mvs_rpc.new_account(self.name, self.password)
        assert (result == 0)

        f = open('./Zac.txt', 'w')
        print >> f, self.lastword()
        f.close()

        result, _ = mvs_rpc.new_address(self.name, self.password, 9)
        assert (result == 0)
        result, self.addresslist = mvs_rpc.list_addresses(self.name, self.password)
        assert (result == 0)
        assert (len(self.addresslist) == 10)
        self.addresslist.reverse()
        return 0, "success"

    def delete(self):
        if (not self.mnemonic) and os.path.exists('./Zac.txt'):
            with open('./Zac.txt') as f:
                lastword = f.read()
                self.mnemonic = [lastword.strip()]
        return Role.delete(self)


homedir = os.path.dirname( os.path.realpath(__file__) )
keystoredir = 'resource/keystore'

#注意， 此处粘贴的listaddresses的结果，与实际address的顺序相反，因此最后一个地址才是第一个

Alice = Role("Alice", "notice judge certain company novel quality plunge list blind library ride uncover fold wink biology original aim whale stand coach hire clinic fame robot",
         [
             "MECD5XzG4Z1eQSyYwZB3YyK3qmct33jT7c",
             "MAhmaLbfLFFMQF88xWAqibDaPfQfYqpv8Y",
             "MNWyXfH6pD1RwfeBiRRqm5eyiu3yAyxyvC",
             "MFjNXDxz8mnKfdTHUjyhLoVMsKY7MixV3V",
             "MLixg7rxKmtPj9DT9wPKSy6WkJkoUDWUSv",
             "MG4kcurE89NPhKX24PgntbTway7sPGmsU4",
             "M9qeDursSJsxK9nHzTcRMnqpe78YXCKv48",
             "MP5FoYQHiEQ52pcEURkaYmuqZMnYHNAZ83",
             "M9L3ipy3Hcf6kdvknU3mH7mwH9ER3uCziu",
             "MLasJFxZQnA49XEvhTHmRKi2qstkj9ppjo"
         ],
        os.path.join(homedir, keystoredir, "Alice.json"),
     )
Bob = Role("Bob", "umbrella social junk engine slender slam adult piece van eight high marriage honey clerk fox input perfect super net refuse connect kick retire sight",
        [
            "MSGA7so2bwjYRpycD7fSUiBmQbjwhFgxmU",
            "MSqVRTpY6r3deZckkHG5rdvfaeyrKtZkBb",
            "MQJ3PCshNdkLvqdF9xu1wf4AssKssKEs3d",
            "MA7yi8rAapTavi8vMnBB8smBiXJVqZVUEH",
            "MGEupYHnCqEXYB5QTsaiySAeHARBqoqRq9",
            "MAZNqQZdeVasSYPu9uFXpnt4JKq7VGYV7h",
            "MQoQa3PPTGZyQDapjCmA39aKBmdRWTQTii",
            "MHcHpynXXgNcVkBr34UNyK57KWwXAeyCSq",
            "MENcHb2KfxjkobX9zTBsJLJ8QdeVW1RteP",
            "MRyes39YXS3MJLf2fNVTtriVRY93v5HARr"
        ],
        os.path.join(homedir, keystoredir, "Bob.json"),
    )
Cindy = Role("Cindy", "small wreck custom end misery aspect spin cricket secret jar goose deliver lens siren wife stick weird enroll excess spin sure wheat april iron",
        [
            "MJyvvycyGvtJdgwaJvHgckbpcwPuC8EuhY",
            "MBStPH1pHHfuwwq4fQQWHjJYFHjsTbYV83",
            "MVdmWZQdKxDTo6mum8N2j8v88Gj2ivexmb",
            "MDNWXUEReJHNdG21JcWYAtaxRKRedu5WNw",
            "MLJoSoK66j7pq94XGQC2sRSUyxErbarCNd",
            "MV2oZAqY6nLXFh2W9YvL2mXE38k3vKoqyt",
            "MHLSeFwxQoG9unD5ynXrf66Qbihe6FDUQZ",
            "MP5RwD3YHQu9r3sAqwzZYY4SkfP9BDEu4W",
            "MM3JMm5LWt3hvhsG63FdhwEiH9kCgcyudU",
            "MVazverbZySt7XUdBjEgAoADHLc2tGML3o"
        ],
        os.path.join(homedir, keystoredir, "Cindy.json"),
     )
Dale = Role("Dale", "cute board similar kind retreat then permit endorse behind swim ribbon photo enjoy obey warrior wink level topple uphold equip suffer present galaxy cushion",
         [
             "MRFRbfB6S6cAAPVQ9ED81SkFueiqipKZGB",
             "MQRwHf7SUjBRYPKdoAynzhmuW48kWg4Brz",
             "MBTJm2T832DDSYagDwx4jwowG2wiK8Bt6Y",
             "MRKXE22Ztatzu8GKKaxz6aM1A62kQXUKM3",
             "MJ64BWA76d2e8kiF1HM6FYzSRHYDqbkek5",
             "MUrW4vuapsZjDBuXafCoZuiyW8FL3yZUbH",
             "MQsZ5SobDzauZgucz4tK3x3H5jRX8i2xt8",
             "ML4Kdra4mNN2vo1qRZtaVHdhmYnwq1cUYy",
             "MApKhkMJzW6H3FfKwcBd6FZ3wcnbnGne8S",
             "MSKFnUNc1njquyo9vGNgFnE7QuR8ygfA4s"
         ],
        os.path.join(homedir, keystoredir, "Dale.json"),
     )
Eric = Role("Eric", "burst tuition primary deer piece bird nose broccoli wasp mule since bubble ladder discover bleak guide brush kiwi luxury only fringe arrange panic shine",
         [
             "MNeM7HX83myQUzr5cyHGQmaUD73ehnZjkD",
             "ME2ekGYHbV9SFAsERZZXX2L3cttLyYWg4V",
             "MJmuEUwjCJousWwHoDEWTkyHUGgSnwhtNt",
             "MSjPxFJg3GYdgDBF41D8dxnEbDLCmAN7hK",
             "MR1KDdvPe6k9eTutiyb3x8KDTEP22BCmFn",
             "MLniaLzNzTxXHvouFcy1v5Rtjjbcj8d51w",
             "MBj9uxesRQsxWWufjf2LpXsehs3LDsJ1PK",
             "MKZbYVAoRVdUqveG33Wt2dsRfVrjYRM5JU",
             "MJjT9TXtwCnaeF12xvWcSiGNC3EsGSQGQD",
             "MAZcEqWkxCGDRoddMBnAqEYeAHgGzTJ4Zn"
         ],
        os.path.join(homedir, keystoredir, "Eric.json"),
     )
Frank = Role("Frank", "angle toddler glow message cart tired scissors violin wisdom stumble toe opera car danger erupt road tourist spirit prosper menu minimum spirit account garbage",
         [
             "MWcLcop8RniSGyhjQtibxB79AJWwkk5b7C",
             "MFUQzjnoDnT7uEaYqo4nnKUUyXo6fjE2m3",
             "MTSWGXb6LFeJpjpFaG4NrUWo8gad9C37Ud",
             "M8K9qWfesYahsyXEVkep9JGRUGqCTZSPmQ",
             "MS3uvnk7JPenzBShJ2wrNE7yW6uwCHKvcB",
             "MQDLNa6gAkyvVyp6KfpoPRTryv1t9PzB5z",
             "MRiR4ey6vSHYqdj5eVdaeEy5UkHM5zYXZQ",
             "MEsKFY6nMx5VJ1VVWaCTii6ghQPuKwXGQM",
             "M9T5X7wWd2bGJT3BSs6FBpfEHdu5tCgsPh",
             "MDdrddED3NKSQoat6AGUojZ1bA5hoxmE1P"
         ],
        os.path.join(homedir, keystoredir, "Frank.json"),
     )

Zac = NewGuy("Zac")

__all__ = ["Alice", "Bob", "Cindy", "Dale", "Eric", "Frank", "Zac"]