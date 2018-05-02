#!/usr/bin/python
# -*- coding: utf-8 -*-

class Asset:
    def __init__(self, symbol):
        self.symbol = symbol

        self.address = None
        self.status = None
        self.description = None
        self.decimal_number = None

        self.issuer = None
        self.is_secondaryissue = None
        self.secondaryissue_threshold = None

        self.quantity = None
        self.maximum_supply = None

    @classmethod
    def init(cls, json_report):

        obj = Asset(json_report['symbol'])
        obj.address = json_report['address']
        obj.status = json_report['status']
        obj.description = json_report['description']
        obj.decimal_number = json_report['decimal_number']

        obj.issuer = json_report['issuer']
        obj.is_secondaryissue = json_report.get('is_secondaryissue', None)
        obj.secondaryissue_threshold = json_report['secondaryissue_threshold']

        obj.quantity = json_report['quantity']
        obj.maximum_supply = json_report.get('maximum_supply', None)

        return obj

class Attachment:
    def __init__(self):
        self.type = "" # etp/message/...
        self.content = ""

    @classmethod
    def from_json(cls, json_report):
        a = cls()

        a.type = json_report["type"]
        if a.type == "message":
            a.content = json_report["content"]

        return a

class PrevOutput:
    def __init__(self):
        self.index = 0
        self.hash = ""

    @classmethod
    def from_json(cls, json_report):
        p = cls()
        p.index = json_report["index"]
        p.hash = json_report["hash"]

        return p

class Input:
    def __init__(self):
        self.previous_output = None
        self.script = ""
        self.sequence = 0
        self.address = ""

    @classmethod
    def from_json(cls, json_report):
        i = cls()
        i.previous_output = PrevOutput.from_json(  json_report["previous_output"] )
        i.script = json_report["script"]
        i.sequence = json_report["sequence"]
        if i.previous_output.index <> 0xFFFFFFFF:
            i.address = json_report["address"]

        return i

class Output:
    def __init__(self):
        self.index = 0
        self.script = ''
        self.value = 0
        self.attachment = None
        self.address = ""
        self.locked_height_range = 0

    @classmethod
    def from_json(cls, json_report):
        o = cls()
        o.index = json_report["index"]
        o.script = json_report["script"]
        o.value = json_report["value"]
        o.attachment = Attachment.from_json(json_report["attachment"])
        o.address = json_report["address"]
        o.locked_height_range = json_report["locked_height_range"]

        return o

class Transaction:
    def __init__(self):
        self.inputs = []
        self.outputs = []
        self.hash = ""
        self.lock_time = 0
        self.height = 0
        self.version = 0

    @classmethod
    def from_json(cls, json_report):
        tx = cls()
        for input in json_report['inputs']:
            tx.inputs.append(  Input.from_json(input)  )

        for output in json_report['outputs']:
            tx.outputs.append(  Output.from_json(output)  )

        tx.hash = json_report['hash']
        tx.lock_time = json_report['lock_time']
        tx.height = json_report['height']
        tx.version = json_report['version']

        return tx
