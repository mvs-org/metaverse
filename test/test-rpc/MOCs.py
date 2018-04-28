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


