#!/usr/bin/python
# -*- coding: utf-8 -*-
import os, time
from utils import mvs_rpc, common

def get_timestamp():
    return time.strftime('%Y%m%d.%H%M%S', time.localtime(time.time()))

class Role:
    def __init__(self, name, mnemonic, addresslist, keystore_file):
        self.name = name
        self.password = name[0] + "123456"
        self.mnemonic = mnemonic.split()
        self.addresslist = addresslist
        self.keystore_file = keystore_file

        self.addresslist.reverse()
        self.did_symbol = name+".DID"
        self.asset_symbol = name+".ASSET." + get_timestamp()
        self.multisig_addresses = {} # desc : multisig-addr

    def lastword(self):
        return self.mnemonic[-1]

    def mainaddress(self):
        return self.addresslist[0]

    def create(self):
        '''
        create account by importkeyfile
        '''
        return mvs_rpc.import_keyfile(self.name, self.password, self.keystore_file)

    def dump_keyfile(self, path):
        return mvs_rpc.dump_keyfile(self.name, self.password, self.lastword(), path)

    def delete(self):
        '''
        delete account by deleteaccount
        '''
        return mvs_rpc.delete_account(self.name, self.password, self.lastword())

    def issue_did(self):
        '''
        issue did to the main address.
        '''
        return mvs_rpc.issue_did(self.name, self.password, self.mainaddress(), self.did_symbol)

    def issue_asset(self):
        '''
        issue asset to the main address.
        '''
        result, message = mvs_rpc.create_asset(self.name, self.password, self.asset_symbol, 100)
        assert (result == True)
        result, message = mvs_rpc.issue_asset(self.name, self.password, self.asset_symbol)
        assert (result == True)

    def mining(self, times=1):
        '''
        use the mainaddress to mining x times.
        do mining to get the main address rich.
        '''
        from ethereum.pow.ethpow import mine
        result, message = mvs_rpc.set_miningaccount(
            self.name,
            self.password,
            self.mainaddress()
        )
        assert(result == True)

        def __mine__():
            header_hash, seed_hash, boundary = mvs_rpc.eth_get_work()
            height, difficulty = mvs_rpc.get_info()

            rounds = 100
            nonce = 0
            while True:
                bin_nonce, mixhash = mine(block_number=height + 1, difficulty=difficulty, mining_hash=header_hash,
                                          rounds=rounds, start_nonce=nonce)
                if bin_nonce:
                    break
                nonce += rounds
            return bin_nonce, '0x' + common.toString(header_hash), '0x' + common.toString(mixhash)

        for i in xrange(times):
            bin_nonce, header_hash, mix_hash = __mine__()
            result, message = mvs_rpc.eth_submit_work('0x' + common.toString(bin_nonce), header_hash, mix_hash)
            assert (result == True)

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
        assert (result == True)
        assert(message["description"] == description)
        self.multisig_addresses[description] = message["address"]
        return message["address"]

    def get_publickey(self, address):
        result, message = mvs_rpc.get_publickey(self.name, self.password, address)
        assert (result == True)
        return message["public-key"]

    def get_multisigaddress(self, description):
        return self.multisig_addresses[description]


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

__all__ = ["Alice", "Bob", "Cindy", "Dale", "Eric", "Frank"]