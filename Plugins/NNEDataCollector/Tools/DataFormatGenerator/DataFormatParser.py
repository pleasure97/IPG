import re
from dataclasses import dataclass, field
from typing import Optional 

# UE & C++ serialization mapping table 
UE_TYPE_MAP = {
    'float' : {'fmt': '%f', 'serialize': '{name}'},
    'int32':   {'fmt': '%d',   'serialize': '{name}'},
    'bool':    {'fmt': '%d',   'serialize': '({name} ? 1 : 0)'},
    'FVector': {'fmt': '%f,%f,%f', 'serialize': '{name}.X,{name}.Y,{name}.Z'},
    'FString': {'fmt': '%s',   'serialize': '*{name}'},
}

@dataclass 
class FieldDef:
    type_name : str
    field_name : str
    label_expr : Optional[str]=None 
    
    @property
    def is_label(self) -> bool:
        return self.label_expr is not None 
        
    @property 
    def fmt(self) -> str:
        return UE_TYPE_MAP.get(self.type_name, {}).get('fmt', '%s')
        
    @property 
    def serialize_expr(self) -> list[str]:
        template = UE_TYPE_MAP.get(self.type_name, {}).get('serialize', '{name}')
        return template.format(name=self.field_name)
        
    @property 
    def csv_columns(self) -> list[str]:
        if self.type_name == 'FVector':
            return [f'{self.field_name}X', f'{self.field_name}Y', f'{self.field_name}Z'}
        return [self.field_name]
        
@dataclass
class RecordDef:
    name : str
    fields : list[FieldDef] = field(default_factory=list)
    label_field : Optional[FieldDef] = None 
    
    @property 
    def file_name(self) -> str:
        return f'{self.name}.csv'
        
    @property 
    def header_columns(self) -> list[str]:
        columns = []
        for field in self.fields:
            columns.extend(field.csv_columns)
        if self.label_field:
            columns.append('Label')
        return columns 
        
    @property 
    def struct_name(self) -> str:
        return f'F{self.name}Record'
        
class NNEDataFormatParser:
    def __init__(self):
        self.records : list[RecordDef] = []
    
    def parse_data_format(self, path : str):
        current_record : Optional[RecordDef] = None 
        
        with open(path, 'r') as f:
            for raw_line in f:
                line = raw_line.strip() 
                # skip blank and annotation line
                if not line or line.startswith('#'):
                    continue
                # parse record block 
                if line.startswith('record'):
                    name = line.split()[1]
                    current_record = RecordDef(name=name)
                    self.records.append(current_record)
                    continue 
                # parse field
                if current_record is None:
                    continue 
                if line.startswith('label '):
                    label_expr = line[len('label '):].strip() 
                    label_field = FieldDef(
                        type_name='label', 
                        field_name='label', 
                        label_expr=label_expr)
                    current_record.label_field = label_field
                else:
                    parts = line.split()
                    if len(parts) >= 2:
                        current_record.fields.append(FieldDef(type_name=parts[0], field_name=parts[1]))
                  
        