import re

class Solution:

    def isValid(self, code: str) -> bool:
        code = re.sub(r'<!\[CDATA\[.*?\]\]>', '-', code)
        prev = None
        while code != prev:
            print(code)
            prev = code
            code = re.sub(r'<([A-Z]{1,9})>[^<]*</\1>', 't', code)
        return code == 't'


Solution().isValid("<DIV>This is the first line <![CDATA[<div>]]></DIV>")



        