"""
1. 템플릿이 되는 .h, .cpp 파일을 읽어들인다.
2. 원래 클래스 이름을 새 클래스 이름으로 변경
3. 새 파일이름으로 저장
"""

import sys

if len(sys.argv) < 2:
    print("Usage: python script.py Example_NEWNAME")
    sys.exit(1)

new_name = sys.argv[1]
print(new_name)

template_header_filename = "Example_TEMPLATE.h"
tempalte_cpp_filename = "Example_TEMPLATE.cpp"

with open(template_header_filename, 'r', encoding='utf-8') as file:
    header_content = file.read()

with open(tempalte_cpp_filename, 'r', encoding='utf-8') as file:
    cpp_content = file.read()

header_content = header_content.replace("Example_TEMPLATE", new_name);
cpp_content = cpp_content.replace("Example_TEMPLATE", new_name);

with open(new_name + ".h", 'w', encoding='utf-8') as output_file:
    output_file.write(header_content)

with open(new_name + ".cpp", 'w', encoding='utf-8') as output_file:
    output_file.write(cpp_content)

